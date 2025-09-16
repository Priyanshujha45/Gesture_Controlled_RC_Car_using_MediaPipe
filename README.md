# Gesture_Controlled_RC_Car_using_MediaPipe
Computer vision and Embedded

1. Setup (Lines 1-13)
We import the necessary libraries:

cv2 (OpenCV) for camera access

mediapipe for hand detection

websockets to communicate with the ESP32

2. Hand Detection Configuration (Lines 16-21)
We set up MediaPipe to detect hands with:

max_num_hands=1 (only track one hand)

min_detection_confidence=0.7 (must be 70% sure it's a hand)

3. Counting Fingers (Lines 24-38)
The count_fingers() function checks which fingers are extended

It looks at the position of finger landmarks relative to the wrist

4. Gesture Recognition (Lines 41-63)
get_gesture() maps hand positions to commands:

5 fingers up → "forward"

Fist (0 fingers) → "stop"

Pointing left → "left"

Pointing right → "right"

Thumbs down → "backward"

5. WebSocket Communication (Lines 66-72)
send_command() connects to the ESP32 and sends the command

Uses async/await for smooth communication

6. Main Program (Lines 75-116)
Opens the webcam

Processes each frame to detect hands

Recognizes gestures and sends commands

Displays the camera feed with the current command

Press 'q' to quit
