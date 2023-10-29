Setup
=====

- from https://gitlab.cri.epita.fr/jeremie.graulle/ssie-s9-robot-command create a personal fork
to be able to commit
- clone your fork on your computer:
`git clone git@gitlab.cri.epita.fr:<name>/ssie-s9-robot-command.git`
- open VS code from this fork: code . &
- Build and Run

Step2
=====

Add other sensors in project robot-simu and class RobotCommand and project robot-command class Robot
to be able to use these sensors from robot command.

For each sensor you have to write a kind of command in the main loop to test the current sensor.
And you have to do a commit in your personal fork after completed each sensor and its test.

The list of sensor is:

- IrProximitySensor
- SwitchSensor
- UltrasonicSensor
- SpeedSensor
