#pragma once
#pragma pack

struct CanMotorCmd {
  int id;
  // 4 byte packing
  // int mode;
  double val;
};

class CanMotorController {
public:

  CanMotorController(int id) {
    cmd = { id, 0.0 };
  }

  virtual void command() = 0;

  CanMotorCmd cmd;
};

class TalonFX : public CanMotorController {
public:
  TalonFX(int id)
    : CanMotorController(id) {}

  void command() override {
    Serial.printf("Commanding motor %i with val %f\n", cmd.id, cmd.val);
  }
};