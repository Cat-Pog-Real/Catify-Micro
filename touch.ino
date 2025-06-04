void handle_touch() {
  if (touch.getSamples(&ti) > 0) {
    num_fingers = ti.count;
    finger_x = ti.x[0];
    finger_y = ti.y[0];
  } else {
    num_fingers = 0;
    finger_x = 0;
    finger_y = 0;
  }

  iVector2 current_pos = iVector2{finger_x, finger_y};
  if (num_fingers > 0) {
      if (old_num_fingers != num_fingers && num_fingers > old_num_fingers) {
        tap_finger_pos.x = finger_x;
        tap_finger_pos.y = finger_y;
      }
    float dist = Find_Squared_Distance(iVector2_to_Vector2(tap_finger_pos), iVector2_to_Vector2(current_pos));
    if (dist > MAX_FINGER_RAD) {
      current_finger_state = DRAG_FINGER;
    } else if (current_finger_state == NONE_FINGER) {
      current_finger_state = TAP_FINGER;
    } else {
      current_finger_state = HOLD_FINGER;
    }
  } else {
    current_finger_state = NONE_FINGER;
  }
  old_num_fingers = num_fingers;
}