#define LGFX_USE_V1
#include <LGFX_AUTODETECT.hpp>
#include <LovyanGFX.hpp>
#include <random>
#include "monitor.h"
#include <sstream>
#include <iomanip>
#include <iostream>

static LGFX lcd(480, 320);

#include <LGFX_AUTODETECT.hpp>
#define max(a,b) ((a) > (b) ? (a) : (b))
#define SPEED 40

enum class StringCode {
    idle,
    growing,
    finished_growing,
    stimulating,
    error
};

StringCode hashString(const std::string str) {
    if (str == "Idle")             return StringCode::idle;
    if (str == "Growing")          return StringCode::growing;
    if (str == "Finished Growing") return StringCode::finished_growing;
    if (str == "Stimulating")     return StringCode::stimulating;
    return StringCode::error;
}

static LGFX_Sprite topBar(&lcd), bottomBar(&lcd), actuator(&lcd), gear(&lcd), data(&lcd), muscleIcon(&lcd);

std::string status = "Growing";
uint32_t error_flash_counter = 1e3;
bool error_flash_state = false;
unsigned long lastTime = 0;
unsigned long deltaTime = 0;
float angle = 360;
float distance;
float eta;
float speed;
std::string previous_status;
std::string error;
std::string selected_wave = "Square";
int frequency = 1000;
float voltage = 0.0f;

float required_distance = 3000.0f; // in micrometers
float required_speed = 10.0f; // in micrometers per hour

int i = 1; // for debugging purposes

float radians(float degrees) {
  return degrees * (3.14159265358979323846f / 180.0f);
}

void drawGear(LGFX_Sprite &sprite, int cx, int cy, int r_inner, int r_outer, int teeth,
              float start_angle_deg = 0, uint32_t color = TFT_WHITE,
              float tooth_gap_ratio = 0.1f, bool fill_center = true) 
{
  float angle_step = 360.0f / (teeth * 2);
  float angle_gap = angle_step * tooth_gap_ratio;

  sprite.fillSprite(lcd.color888(150, 150, 150));

  for (int i = 0; i < teeth; ++i) {
    float angle_outer = start_angle_deg + i * 2 * angle_step;
    float angle_left  = angle_outer - (angle_step - angle_gap);
    float angle_right = angle_outer + (angle_step - angle_gap);

    // Tip flattening factor (how wide the flat top is)
    float flat_factor = 0.3f;
    float flat_angle = angle_step * flat_factor;

    float angle_flat_left = angle_outer - flat_angle;
    float angle_flat_right = angle_outer + flat_angle;

    // Base corners
    float x1 = cx + r_inner * cos(radians(angle_left));
    float y1 = cy + r_inner * sin(radians(angle_left));
    float x4 = cx + r_inner * cos(radians(angle_right));
    float y4 = cy + r_inner * sin(radians(angle_right));

    // Tip corners (flattened)
    float x2 = cx + r_outer * cos(radians(angle_flat_left));
    float y2 = cy + r_outer * sin(radians(angle_flat_left));
    float x3 = cx + r_outer * cos(radians(angle_flat_right));
    float y3 = cy + r_outer * sin(radians(angle_flat_right));

    // Draw the trapezoid tooth
    sprite.fillTriangle(x1, y1, x2, y2, x3, y3, color);
    sprite.fillTriangle(x1, y1, x3, y3, x4, y4, color);
  }

  if (fill_center) {
    sprite.fillCircle(cx, cy, r_inner, color);
  }
}

void drawBar(LGFX_Sprite &sprite, bool top){
  if(top) {
    sprite.fillRect(0, 0, 460, 30, lcd.color888(59, 217, 190));
    sprite.drawString(std::string("Status: " + status).c_str(), 240, 8);
    sprite.pushSprite(10, 10);

    return;
  }

  std::string message;
  int option_buttons = 0;
  uint32_t color;
  switch (hashString(status)) {
    case StringCode::idle:
      option_buttons = 1;
      message = "Start growing";
      color = lcd.color888(59, 217, 190);
      break;
    case StringCode::growing:
      message = "Stop growing";
      color = lcd.color888(255, 0, 0);
      break;
    case StringCode::finished_growing:
      option_buttons = 2;
      message = "Start stimulating";
      color = lcd.color888(59, 217, 190);
      break;
    case StringCode::stimulating:
      message = "Stop stimulating";
      color = lcd.color888(255, 0, 0);
      break;
    default:
      message = "An error has occured!";
      color = lcd.color888(255, 0, 0);
  }

  if(error_flash_counter > 1000) {
    error_flash_counter = 1000;
    error_flash_state = !error_flash_state;
  }

  if(error_flash_state) {
    color = lcd.color888(0, 0, 0);
  }

  if(option_buttons == 0) {
    sprite.fillRect(0, 0, 460, 30, color);
    sprite.setTextColor(TFT_WHITE, color);
    sprite.drawString(message.c_str(), 240, 8);
    sprite.pushSprite(10, 280);

    return;
  } else if (option_buttons == 1) {
    sprite.fillRect(0, 0, 50, 30, TFT_DARKGREEN);
    sprite.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    sprite.drawString("D+", 25, 8);

    sprite.fillRect(55, 0, 50, 30, TFT_RED);
    sprite.setTextColor(TFT_WHITE, TFT_RED);
    sprite.drawString("D-", 80, 8);

    sprite.fillRect(110, 0, 50, 30, TFT_DARKGREEN);
    sprite.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    sprite.drawString("S+", 135, 8);

    sprite.fillRect(165, 0, 50, 30, TFT_RED);
    sprite.setTextColor(TFT_WHITE, TFT_RED);
    sprite.drawString("S-", 190, 8);

    sprite.fillRect(220, 0, 240, 30, color);
    sprite.setTextColor(TFT_WHITE, color);
    sprite.drawString(message.c_str(), 340, 8);

    sprite.pushSprite(10, 280);
  } else if (option_buttons == 2) {
    sprite.fillRect(0, 0, 50, 30, TFT_DARKGREEN);
    sprite.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    sprite.drawString("T+", 25, 8);

    sprite.fillRect(55, 0, 50, 30, TFT_RED);
    sprite.setTextColor(TFT_WHITE, TFT_RED);
    sprite.drawString("T-", 80, 8);

    sprite.fillRect(110, 0, 50, 30, TFT_DARKGREEN);
    sprite.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    sprite.drawString("F+", 135, 8);

    sprite.fillRect(165, 0, 50, 30, TFT_RED);
    sprite.setTextColor(TFT_WHITE, TFT_RED);
    sprite.drawString("F-", 190, 8);

    sprite.fillRect(220, 0, 240, 30, color);
    sprite.setTextColor(TFT_WHITE, color);
    sprite.drawString(message.c_str(), 340, 8);

    sprite.pushSprite(10, 280);
  }
}

void drawActuator(LGFX_Sprite &actuator_sprite, LGFX_Sprite &gear_sprite){
  actuator_sprite.fillRect(0, 0, 180, 180, lcd.color888(150, 150, 150));
  drawGear(gear_sprite, 64, 64, 50, 64, 12, 0, lcd.color888(100, 100, 100), 0.3f);
  actuator_sprite.pushSprite(280, 70);
  gear_sprite.pushSprite(306, 96);
}

void drawData(LGFX_Sprite &data_sprite){
  if (true) { // status != previous_status || status == "Idle") { 
    data_sprite.fillSprite(TFT_BLACK);

    std::string str;
    std::ostringstream oss;

    switch (hashString(status)) {
      case StringCode::idle:
        data_sprite.drawString("Press the button", 0, 10);
        data_sprite.drawString("to start the growing", 0, 30);
        data_sprite.drawString("process!", 0, 50);
        data_sprite.drawString("Distance: ", 0, 100);
        data_sprite.drawString("Speed: ", 0, 120);
        data_sprite.drawString("ETA: ", 0, 160);

        oss << std::fixed << std::setprecision(1) << required_distance << "um";
        data_sprite.drawString(oss.str().c_str(), 115, 100);

        oss.str("");
        oss << std::fixed << std::setprecision(1) << required_speed << "um/h";
        data_sprite.drawString(oss.str().c_str(), 75, 120);

        oss.str("");
        oss << std::fixed << std::setprecision(1) << required_distance / required_speed << "h";
        data_sprite.drawString(oss.str().c_str(), 50, 160);

        break;
      case StringCode::growing:
        eta = (required_distance - distance) / required_speed;
        oss << "Distance: " << std::fixed << std::setprecision(2) << distance << "um";
        data_sprite.drawString(oss.str().c_str(), 0, 0);
        oss.str("");

        oss << "ETA: " << std::fixed << std::setprecision(2) << eta << "h";
        data_sprite.drawString(oss.str().c_str(), 0, 83);
        oss.str("");

        oss << "Speed: " << std::fixed << std::setprecision(2) << speed << "um/h";
        data_sprite.drawString(oss.str().c_str(), 0, 166);
        break;
      case StringCode::finished_growing:
        data_sprite.drawString("The growing process", 0, 0);
        data_sprite.drawString("is completed. Press", 0, 20);
        data_sprite.drawString("the button to start", 0, 40);
        data_sprite.drawString("stimulating!", 0, 60);

        oss << "Wave type: " << selected_wave;
        data_sprite.drawString(oss.str().c_str(), 0, 110);
        oss.str("");

        oss << "Frequency: " << frequency << "Hz";
        data_sprite.drawString(oss.str().c_str(), 0, 140);
        break;
      case StringCode::stimulating:
        oss << "Voltage: " << std::fixed << std::setprecision(2) << voltage << "V";
        data_sprite.drawString(oss.str().c_str(), 0, 83);
        break;
      case StringCode::error:
        data_sprite.drawString("Error!", 0, 73);
        data_sprite.drawString(error.c_str(), 0, 93);
        break;
    }

    data_sprite.pushSprite(10, 70);

    previous_status = status;
  }
}

void setup(void)
{
  lcd.init();
  // lcd.setRotation(3);
  lcd.setTextSize(2); 
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextDatum(top_center);

  topBar.createSprite(460, 30);
  topBar.setTextSize(2);
  topBar.setTextDatum(top_center);
  topBar.setTextColor(TFT_WHITE, lcd.color888(59, 217, 190));
  bottomBar.createSprite(460, 30);
  bottomBar.setTextSize(2);
  bottomBar.setTextDatum(top_center);

  actuator.createSprite(180, 180);
  gear.createSprite(128, 128);

  data.createSprite(240, 180);
  data.setTextSize(2);
  data.setTextDatum(TL_DATUM);
  data.setTextColor(TFT_WHITE);

  // muscleIcon.createSprite(180, 180);
  // muscleIcon.pushImage(0, 0, 180, 180, icon);
  // muscleIcon.pushSprite(30, 30);

  // lcd.draw

  drawBar(topBar, true);
  drawBar(bottomBar, false);

  drawActuator(actuator, gear);

  distance = 1.234;
  eta = 142.54;
  speed = 0.01;
  error = "Motor has no power.";

  drawData(data);
}

void loop(void)
{
  unsigned long now = lgfx::v1::millis();
  deltaTime = now - lastTime;
  lastTime = now;

  if(hashString(status) == StringCode::error) {
    error_flash_counter -= deltaTime;
  }

//   if (now > 20000) status = "Error";
//   else if (now > 15000) status = "Sensing";
//   else if (now > 10000) status = "Finished Growing";
//   else if (now > 5000) status = "Growing";

  // required_distance = 3000.0f - now / 1000.0f; // Simulate changing distance

//   if (now > 1000000000) status = "Stimulating";
//   else if (now > 5000){
//     status = "Growing";
//     speed = required_speed;
//   }

  drawBar(topBar, true);
  drawBar(bottomBar, false);
  drawData(data);

  if(hashString(status) == StringCode::growing) {
    angle += float(deltaTime) / SPEED;
    gear.pushRotateZoom(370, 160, angle, 0.95, 0.95);

    distance = now / 1000.0f * required_speed / 3600; // Simulate distance based on time
  }
}