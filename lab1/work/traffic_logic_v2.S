#define __SFR_OFFSET 0x00

.global start
.global calculate_cross_traffic_red_time
.global calculate_cross_traffic_green_time

.section .text    ; Defines a code section

; dummy start
start:
  RET

; input is GREEN time u8
calculate_cross_traffic_red_time:
  ADD R24, 3
  RET

; input is RED time u8
calculate_cross_traffic_green_time:
  SUB R24, 3
  RET