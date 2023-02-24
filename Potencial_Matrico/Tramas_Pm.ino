void Rx_Data() {

  while (Serial.available()) {

    if (Serial.available() > 0) {
      data = Serial.read();

      if (data == 13) {                                                              // FIN DE LAS TRAMAS
        if (flag_rx_n) {                                                             // CAPTURA TRAMA N
          PM10 += S_PM10.toInt() / 10.0;
          PM25 += S_PM25.toInt() / 10.0;
          S_PM10 = ""; S_PM25 = "";
          muestra++;
        }

        if (flag_rx_p) {
          Ano = S_Ano.toInt();
          Mes = S_Mes.toInt();
          Dia = S_Dia.toInt();
          Hor = S_Hor.toInt();
          Min = S_Min.toInt();
          Error = S_Error.toInt();
          Temperatura = (S_Temp.toInt()) - 6;
          Humedad = S_Hum.toInt();
          S_Ano = ""; S_Mes = ""; S_Dia = ""; S_Hor = ""; S_Min = ""; S_Error = ""; S_Temp = ""; S_Hum = "";
          lcd();

          if (Min == 0) {
            tx_payload_1[0]  = Mes;
            tx_payload_1[1]  = Dia;
            tx_payload_1[2]  = Hor;
            tx_payload_1[3]  = Min;

            var = (PM10 / muestra) * 100;
            tx_payload_1[4] = ((var & 0xff00) >> 8);
            tx_payload_1[5] =  (var & 0x00ff);

            var = (PM25 / muestra) * 100;
            tx_payload_1[6] = ((var & 0xff00) >> 8);
            tx_payload_1[7] =  (var & 0x00ff);

            tx_payload_1[8]  = Error;
            tx_payload_1[9]  = Temperatura;
            tx_payload_1[10] = Humedad;

            flag_ext = true;
          }

          if ((Min == 15) || (Min == 30) || (Min == 45)) {
            if (tx_payload_1[0] != 0) {
              do_send_1(&sendjob);
            }
          }

          if (Error != 0) {
            flag_ext = true;
          }

          if ((Hor == 0) && (Min == 1)) {
            muestra = 0; PM10 = 0; PM25 = 0;
          }
        }

        flag_rx_n = false;
        flag_rx_p = false;
        Separador = 0;

      } else {

        Trama_N();
        Trama_P();

        if (data == 'N') {
          flag_rx_n = true;          // START OF DATA RX
          Separador = 0;
        }

        if (data == 'P') {
          flag_rx_p = true;
          Separador = 0;
        }

      }
    }
  }
}


void Trama_N() {
  if (flag_rx_n) {                                               // RECEIVE DATA FOR TRAMA N

    if ((flag_esp) && (data > 0x29) && (data < 0x40)) {
      flag_esp = false;
      Separador++;
    }

    if (data == ' ') {
      flag_esp = true;
    }

    switch (Separador) {
      case 2:
        S_PM10 += data;
        break;

      case 3:
        S_PM25 += data;
        break;
    }
  }
}


void Trama_P() {

  if (flag_rx_p) {                                              // RECEIVE DATA POR TRAMA P

    if ((flag_esp) && (data > 0x29) && (data < 0x40)) {
      flag_esp = false;
      Separador++;
    }

    if (data == ' ') {
      flag_esp = true;
    }

    switch (Separador) {
      case 1:
        S_Ano += data;
        break;

      case 2:
        S_Mes += data;
        break;

      case 3:
        S_Dia += data;
        break;

      case 4:
        S_Hor += data;
        break;

      case 5:
        S_Min += data;
        break;

      case 8:
        S_Error += data;
        break;

      case 14:
        S_Hum += data;
        break;

      case 15:
        S_Temp += data;
        break;
    }
  }
}

void last_data() {

}
