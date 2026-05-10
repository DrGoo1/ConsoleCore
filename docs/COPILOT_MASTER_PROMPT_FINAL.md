# Copilot Master Prompt — ConsoleCore Final Milestones N–T

Continue the ConsoleCore project using the existing milestone packages A–M.

Implement the final milestones in this order:

1. Milestone N — Production DSP Integration
2. Milestone P — Advanced Analog Modeling
3. Milestone Q — Plugin Ecosystem Completion
4. Milestone O — Visual Console & UX System
5. Milestone R — Hardware-Integrated Capture Lab
6. Milestone S — Release Candidate & Optimization
7. Milestone T — Hardware roadmap documentation only

Hard requirements:
- Preserve realtime safety.
- No heap allocation inside audio callback.
- No blocking locks inside audio callback.
- Keep all built-in console naming legally safe and generic.
- Do not clone proprietary code, JSFX implementations, or branded console behavior claims.
- Maintain Apple Silicon macOS priority.
- Use JUCE/CMake structure.
- Every milestone must include CLI or automated validation where possible.
- Commit frequently with meaningful Git checkpoints.