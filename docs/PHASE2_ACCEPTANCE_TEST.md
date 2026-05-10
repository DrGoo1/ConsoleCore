# Phase 2 Acceptance Test

1. Build the app from a clean clone.
2. Create a 16-channel Loopback device.
3. Route DAW stems to Loopback pairs 1/2, 3/4, 5/6, etc.
4. Select ConsoleCore as the receiving CoreAudio app.
5. Confirm pass-through mode still works.
6. Enable Console Engine mode.
7. Confirm rail sag meter increases when more stems play.
8. Confirm Drive and Stress are audible but level-compensated enough to avoid runaway gain.
9. Confirm bypass returns to clean pass-through.
10. Run high-level sine/sweep/noise tests and verify no NaN/Inf output.
