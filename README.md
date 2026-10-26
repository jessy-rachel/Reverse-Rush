# Reverse Rush — Arduino Joystick Reaction Game

**Reverse Rush** is a reflex and direction-processing game built around an **Arduino Uno R3**, an **8×8 MAX7219 LED matrix**, a two-axis analog joystick and a buzzer.

![Reverse Rush physical joystick and LED matrix game](images/gameplay.jpg)

*Actual Arduino/joystick/LED-matrix gameplay prototype.*

## Gameplay implemented in the supplied sketch

1. The matrix displays a randomly chosen directional arrow.
2. The player must move the joystick in the **opposite** direction and hold it long enough to register.
3. A successful response adds a point; a failure costs one life.
4. The game starts with **three lives**, displays success/failure icons, and shows the score at game over before restarting.

> **Code note:** The initial comment in the source mentions left/right staying the same, but the implemented `oppositeOf()` function reverses **all four directions**. This README documents the current *implemented behavior*. Update the function if the intended rules are different.

## Components and wiring

| Component | Arduino Uno R3 pin |
|---|---|
| MAX7219 DIN | D12 |
| MAX7219 CLK | D11 |
| MAX7219 CS/LOAD | D10 |
| Joystick X-axis | A0 |
| Joystick Y-axis | A1 |
| Joystick push-button | D2 (configured with pull-up; not presently part of gameplay) |
| Buzzer | D8 |
| MAX7219 power | Appropriate 5V and GND connections |
| Joystick power | 5V and GND |

Use a suitable MAX7219 module; verify the device's current draw, grounding and module orientation.

## Software

1. Install Arduino IDE and the **LedControl** library.
2. Open [`src/Reverse_Rush_final.ino`](src/Reverse_Rush_final.ino).
3. Select **Arduino Uno** and the serial port, then upload.
4. Leave the joystick centered during startup while it calibrates its center position.
5. React to arrows by moving the joystick in the *opposite* direction.

## Configurable game parameters

| Constant | Current value | Purpose |
|---|---|---|
| `ROUND_TIME_MS` | 900 ms | Time allowed per response |
| `HOLD_OK_MS` | 180 ms | Required correct-direction hold |
| `LIVES_START` | 3 | Starting lives |
| `DEADZONE` | 140 | Joystick input deadzone (later clamped in setup) |
| `MATRIX_ORIENTATION` | 0 | Adjust matrix rotation: 0/90/180/270 |

The matrix includes arrow icons, check/cross feedback, and score-rendering logic with one-digit and compact two-digit layouts. Displayed scores are capped at 99, although the internal score variable can continue increasing.

## Repository structure

- `src/Reverse_Rush_final.ino` — original Arduino sketch
- `images/gameplay.jpg` — gameplay prototype photo
- `README.md` and `.gitignore`

## Future improvements

- Add a short gameplay video showing arrow prompts and reactions.
- Make joystick/button settings user-configurable.
- Add a start/restart button and selectable difficulty modes.
- Align the introductory source comment with the implemented direction rules.
