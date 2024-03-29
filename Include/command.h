#ifndef COMMAND_H_
#define COMMAND_H_

#define RSP_FOTA_OK "{\"RES\":\"FOTA_OK\"}"
#define RSP_FOTA_FAIL "{\"RES\":\"FOTA_FAIL\"}"

#define RSP_SOS1_OK "{\"RES\":\"SOS1_OK\"}"
#define RSP_SOS2_OK "{\"RES\":\"SOS2_OK\"}"

#define RSP_SHUTDOWN_OK "{\"RES\":\"SHUTDOWN_OK\"}"
#define RSP_SHUTDOWN_FAIL "{\"RES\":\"SHUTDOWN_FAIL\"}"
#define RSP_SPEAKER_ON_OK "{\"RES\":\"SPEAKER_ON_OK\"}"
#define RSP_SPEAKER_OFF_OK "{\"RES\":\"SPEAKER_OFF_OK\"}"

#define RSP_LIGHT_ON_OK "{\"RES\":\"LIGHT_ON_OK\"}"
#define RSP_LIGHT_OFF_OK "{\"RES\":\"LIGHT_OFF_OK\"}"

#define RSP_SPEAKER_ON_FAIL "{\"RES\":\"SPEAKER_ON_FAIL\"}"

#define RSP_POWER_ON_OK "{\"RES\":\"POWER_ON_OK\"}"
#define RSP_POWER_ON_FAIL "{\"RES\":\"POWER_ON_FAIL\"}"

#define RSP_POWER_OFF_OK "{\"RES\":\"POWER_OFF_OK\"}"
#define RSP_POWER_OFF_FAIL "{\"RES\":\"POWER_OFF_FAIL\"}"

#define RSP_CLOSE_OK "{\"RES\":\"CLOSE_OK\"}"
#define RSP_CLOSE_FAIL "{\"RES\":\"CLOSE_FAIL\"}"

#define RSP_GPS "{\"RES\":\"GPS_FAIL\"}"

#define CMD_KTTK "AT+CUSD=1,\"*101#\",15\r\n"

#define RSP_POWER_ON "{\"POWER_STT\":\"ON\"}"
#define RSP_POWER_OFF "{\"POWER_STT\":\"OFF\"}"


#endif