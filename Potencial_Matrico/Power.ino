// --------------------- POWER SETTING --------------//

void AX192_init() {
  Wire.begin(21, 22);
  axp.begin(Wire, AXP192_SLAVE_ADDRESS);

  axp.setDCDC1Voltage(3300);                                // Serial
  axp.setDCDC2Voltage(0);                                   // Unused
  axp.setLDO2Voltage(3300);                                 // LoRa
  axp.setLDO3Voltage(3300);                                 // GPS

  axp.setVWarningLevel1(3600);
  axp.setVWarningLevel2(3800);
  axp.setPowerDownVoltage(3000);

  axp.setTimeOutShutdown(false);                            // no automatic shutdown
  axp.setTSmode(AXP_TS_PIN_MODE_DISABLE);                   // TS pin mode off to save power
  axp.setShutdownTime(AXP_POWER_OFF_TIME_4S);
  axp.setStartupTime(AXP192_STARTUP_TIME_1S);

  axp.adc1Enable(AXP202_BATT_VOL_ADC1, true);               // Turn on ADCs.
  axp.adc1Enable(AXP202_BATT_CUR_ADC1, true);
  axp.adc1Enable(AXP202_VBUS_VOL_ADC1, true);
  axp.adc1Enable(AXP202_VBUS_CUR_ADC1, true);

  axp.setChargeControlCur(AXP1XX_CHARGE_CUR_450MA);         // Start charging the battery if it is installed.
  axp.setChargingTargetVoltage(AXP202_TARGET_VOL_4_2V);
  axp.enableChargeing(true);
  axp.setChgLEDMode(AXP20X_LED_OFF);

  axp.setBackupChargeCurrent(AXP202_BACKUP_CURRENT_100UA);  // Keep the on-board clock (& GPS) battery topped up.
  axp.setBackupChargeVoltage(AXP202_BACKUP_VOLTAGE_3V0);
  axp.setBackupChargeControl(true);

  axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);              // Serial
  axp.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);             // Unused
  axp.setPowerOutPut(AXP192_LDO2,  AXP202_ON);              // LoRa
  axp.setPowerOutPut(AXP192_LDO3,  AXP202_OFF);             // GPS
  axp.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);             // Unused
}
