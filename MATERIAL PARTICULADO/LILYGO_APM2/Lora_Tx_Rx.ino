
// ---------------------------------- TX FRAME LORAWAM -------------------------------

void do_send(osjob_t* j) {

  for (uint8_t x = 0; x < 5; x++) {
    digitalWrite(Led, LOW);
    delay(25);
    digitalWrite(Led, HIGH);
    delay(25);
  }

  tx_payload[0]  = 2 ;     //Mes;
  tx_payload[1]  = 27;     //Dia;
  tx_payload[2]  = Hor;
  tx_payload[3]  = Min;

  var = (Pm10 / Muestra) * 100;
  tx_payload[4] = ((var & 0xff00) >> 8);
  tx_payload[5] =  (var & 0x00ff);

  var = (PM25 / Muestra) * 100;
  tx_payload[6] = ((var & 0xff00) >> 8);
  tx_payload[7] =  (var & 0x00ff);

  tx_payload[8]  = Err;
  tx_payload[9]  = (Temp*1.8) + 32; //Temperatura;
  tx_payload[10] = Hum *1.2585;     //Humedad;
  tx_payload[11] = Pre / 5;         //Humedad;

  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    LMIC_setTxData2(1, tx_payload, 12, 1);
  }

}

void do_send_1(osjob_t* j) {

  for (uint8_t x = 0; x < 5; x++) {
    digitalWrite(Led, LOW);
    delay(25);
    digitalWrite(Led, HIGH);
    delay(25);
  }

  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    LMIC_setTxData2(1, tx_payload_1, 12, 0);
  }

}


void onEvent (ev_t ev) {
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_RFU1:
      ||     Serial.println(F("EV_RFU1"));
      ||     break;
    */
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      if (LMIC.txrxFlags & TXRX_ACK) {
        flag_ack = true;
      }

      if (LMIC.dataLen) {
        lenght = LMIC.dataLen;
        for (int i = 0; i < LMIC.dataLen; i++) {
          prueba[i] = LMIC.frame[LMIC.dataBeg + i];
        }
        //flag_rx = true;
      }

      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_SCAN_FOUND:
      ||    Serial.println(F("EV_SCAN_FOUND"));
      ||    break;
    */
    case EV_TXSTART:
      Serial.println(F("EV_TXSTART"));
      break;
    case EV_TXCANCELED:
      Serial.println(F("EV_TXCANCELED"));
      break;
    case EV_RXSTART:
      /* do not print anything -- it wrecks timing */
      break;
    case EV_JOIN_TXCOMPLETE:
      Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
      break;

    default:
      Serial.print(F("Unknown event: "));
      Serial.println((unsigned) ev);
      break;
  }
}
