

// ------------------------------------------------- INIT LORA WAN --------------------------------------------------

void Init_Lora() {

  str_key("125398ED5B2DD04005424AADC5208A92");         // LEER PARAMETROS LORAWAN
  memcpy_P(NWKSKEY, KEY, 16);
  str_key("977FDF80D6F2F35F0FDEB52AA4D8DDF0");
  memcpy_P(APPSKEY, KEY, 16);
  str_DEVADDR("30518A3C");

  SPI.begin(5, 19, 27, 18);
  os_init();
  LMIC_reset();

  LMIC_setAdrMode(0);                                               // Enable or disable data rate adaptation
  LMIC_selectSubBand(1);                                            // AU915
  LMIC_setLinkCheckMode(0);                                         // Disable link check validation
  LMIC.dn2Dr = DR_SF9;                                              // TTN uses SF9 for its RX2 window.
  LMIC_setDrTxpow(DR_SF12, 14);
  LMIC_clrTxData ();
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
}


// ------------------------------------------------- KEY'S TO ARRAY ---------------------------------------------------

void str_key(String cadena) {
  uint8_t cto = 0;
  uint8_t str_len = cadena.length() + 1;
  char char_array[str_len];
  cadena.toCharArray(char_array, str_len);

  for (uint8_t x = 0; x < str_len - 1; x = x + 2) {
    uint8_t valor = 0;
    if (char_array[x] < 58) {
      valor = char_array[x] - 48;
    } else {
      valor = char_array[x] - 55;
    } valor = valor * 16;

    if (char_array[x + 1] < 58) {
      valor = valor + char_array[x + 1] - 48;
    } else {
      valor = valor + char_array[x + 1] - 55;
    }

    KEY[cto] = valor;
    cto++;
  }
}

// --------------------------------------------------- KEY TO DEVADDR -----------------------------------------------------

void str_DEVADDR(String cadena) {
  DEVADDR = 0;
  uint8_t str_len = cadena.length() + 1;
  char char_array[str_len];
  cadena.toCharArray(char_array, str_len);

  uint32_t valor;
  uint8_t des[] = {28, 24, 20, 16, 12, 8, 4, 0};
  for (uint8_t x = 0; x < str_len - 1; x++) {
    if (char_array[x] < 58) {
      valor = char_array[x] - 48;
    } else {
      valor = char_array[x] - 55;
    }
    DEVADDR += (valor << des[x]);
  }
}
