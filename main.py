# Import necessary libraries
import cv2  # For camera access
import mediapipe as mp  # For hand detection
import asyncio  # For WebSocket communication
import websockets  # To send commands to ESP32

## ========================================
## SETUP: Configure Hand Detection
## ========================================

# Initialize MediaPipe hand detection
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(
    max_num_hands=1,  # Only detect one hand
    min_detection_confidence=0.7  # How sure we need to be that it's a hand
)

# For drawing hand landmarks (optional visual)
mp_drawing = mp.solutions.drawing_utils

## ========================================
## GESTURE DETECTION
## ========================================

def count_fingers(landmarks):
    """Count how many fingers are extended (0-5)"""
    finger_tips = [8, 12, 16, 20]  # Index, Middle, Ring, Pinky tips
    thumb_tip = 4
    wrist = 0
    
    fingers_up = 0
    
    # Check thumb (right hand only)
    if landmarks.landmark[thumb_tip].x < landmarks.landmark[thumb_tip-1].x:
        fingers_up += 1
    
    # Check other fingers
    for tip in finger_tips:
        if landmarks.landmark[tip].y < landmarks.landmark[tip-2].y:
            fingers_up += 1
    
    return fingers_up

def get_gesture(landmarks):
    """Determine which command to send based on hand position"""
    fingers = count_fingers(landmarks)
    
    # Open palm (all fingers extended)
    if fingers == 5:
        return "forward"
    
    # Closed fist (no fingers extended)
    elif fingers == 0:
        return "stop"
    
    # Pointing left (only index finger extended)
    elif fingers == 1 and landmarks.landmark[8].x < landmarks.landmark[0].x - 0.1:
        return "left"
    
    # Pointing right (only index finger extended)
    elif fingers == 1 and landmarks.landmark[8].x > landmarks.landmark[0].x + 0.1:
        return "right"
    
    # Thumbs down (only thumb extended downward)
    elif fingers == 1 and landmarks.landmark[4].y > landmarks.landmark[2].y:
        return "backward"
    
    return None  # No recognized gesture

## ========================================
## WEBSOCKET COMMUNICATION
## ========================================

async def send_command(command):
    """Send a command to the ESP32 via WebSocket"""
    try:
        async with websockets.connect("ws://192.168.4.1/ws") as ws:
            await ws.send(command)
            print(f"Sent: {command}")
    except Exception as e:
        print(f"Couldn't send command: {e}")

## ========================================
## MAIN PROGRAM
## ========================================

def main():
    # Open webcam
    cap = cv2.VideoCapture(0)
    last_command = None
    
    while True:
        # Read camera frame
        success, frame = cap.read()
        if not success:
            print("Can't access camera!")
            break
        
        # Flip frame so it's like a mirror
        frame = cv2.flip(frame, 1)
        
        # Convert to RGB (MediaPipe needs RGB)
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        
        # Detect hands
        results = hands.process(rgb_frame)
        
        # If hand is detected
        if results.multi_hand_landmarks:
            for hand_landmarks in results.multi_hand_landmarks:
                # Draw hand landmarks (optional)
                mp_drawing.draw_landmarks(
                    frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)
                
                # Get gesture command
                command = get_gesture(hand_landmarks)
                
                # Send new commands only when they change
                if command and command != last_command:
                    asyncio.run(send_command(command))
                    last_command = command
        
        # Display the command on screen
        cv2.putText(frame, f"Command: {last_command}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        
        # Show the camera feed
        cv2.imshow("Hand Gesture Control", frame)
        
        # Press 'q' to quit
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    # Clean up
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()