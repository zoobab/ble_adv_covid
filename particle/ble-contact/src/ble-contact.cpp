/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/Users/nams/Documents/projects/covid/particle/ble-contact/src/ble-contact.ino"
/*
 * Project ble-contact
 * Description: Bluetooth scan
 * Author:  SaeWoo Nam
 * Date:  April 20, 2020
 */
#include "Particle.h"
#include "config.h"

void sprintf_buffer(uint8_t *data, uint8_t length);
void scanResultCallback(const BleScanResult *scanResult, void *context);
void gap_params_init();
void advertising_init(void);
void calc_RPI(uint8_t *data);
void setup();
void loop();
#line 10 "/Users/nams/Documents/projects/covid/particle/ble-contact/src/ble-contact.ino"
BleAdvertisingData advData;
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];

// For debugging only
// SYSTEM_MODE(MANUAL);

// Enable thread
// SYSTEM_THREAD(ENABLED);

// Stores the most recent data related to the device we're looking for
int8_t lastRSSI;
system_tick_t lastSeen = 0;

// The payload going to the cloud
String status;

// For logging
SerialLogHandler logHandler(115200, LOG_LEVEL_ERROR, {
    { "app", LOG_LEVEL_TRACE }, // enable all app messages
});

static char m_tx_buffer[64];

void sprintf_buffer(uint8_t *data, uint8_t length) {
  uint8_t size = 0;
  if (length>0) {
    for (uint8_t i=0; i<length; i++) {
      //if (i>0) size+=sprintf(m_tx_buffer+size, "-");
      size+= sprintf(m_tx_buffer+size, "%02x", data[i]);
    }
  }
}

void scanResultCallback(const BleScanResult *scanResult, void *context) {
  BleUuid uuids[4];
  int uuidsAvail = scanResult->advertisingData.serviceUUID(uuids,sizeof(uuids)/sizeof(BleUuid));
  // Print out mac info
  BleAddress addr = scanResult->address;
  // Log.trace("Num of UUIDs: %d", uuidsAvail);

  for(int i = 0; i < uuidsAvail; i++){
    // Print out the UUID we're looking for
    if( uuids[i].shorted() == CONTACT_UUID ) {
        // Log.trace("UUID: %x", uuids[i].shorted());
        Log.trace("RSSI: %d MAC: %02X:%02X:%02X:%02X:%02X:%02X", 
                  scanResult->rssi, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
        //Log.trace("RSSI: %ddBm", scanResult->rssi);
        lastSeen = millis();
        lastRSSI = scanResult->rssi;
        // Stop scanning
        // BLE.stopScanning();

        return;
    }
  }
}

void gap_params_init() {
    unsigned int err_code;
    ble_gap_privacy_params_t prvt_conf;
    memset(&prvt_conf, 0, sizeof(prvt_conf));
    prvt_conf.privacy_mode = BLE_GAP_PRIVACY_MODE_DEVICE_PRIVACY;
    prvt_conf.private_addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE ;
    prvt_conf.private_addr_cycle_s = MAC_ADDR_ROTATE_TIME_S;
    err_code = sd_ble_gap_privacy_set(&prvt_conf);
}

void advertising_init(void)
{
    // uint32_t      err_code;
    // ble_advdata_t advdata;
    // uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
    
    // flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    
    /*
    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
    m_adv_params.p_peer_addr     = NULL;    // Undirected advertisement.
    m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval        = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.duration        = 0;       // Never time out.
    */
    BLE.setAdvertisingType(BleAdvertisingEventType::NON_CONNECTABLE_NON_SCANABLE_UNDIRECTED);
    BleAddress defaultAddr = BLE.address();
    BLE.address().type(BleAddressType::RANDOM_PRIVATE_NON_RESOLVABLE);
    int8_t txPower;
    txPower = BLE.txPower(&txPower);
    m_enc_advdata[0] = 0x2;  // Set TX_POWER
    m_enc_advdata[1] = 0xA;
    m_enc_advdata[2] = txPower; 
    m_enc_advdata[3] = 0x02;  // SET Flags
    m_enc_advdata[4] = 0x01;
    m_enc_advdata[5] = 0x06;
    m_enc_advdata[6] = 0x03;  // Set contact UUID
    m_enc_advdata[7] = 0x03;
    m_enc_advdata[8] = 0x6F;
    m_enc_advdata[9] = 0xFD;
    m_enc_advdata[10] = 0x13;
    m_enc_advdata[11] = 0x16;
    m_enc_advdata[12] = 0x6F;
    m_enc_advdata[13] = 0xFD;
    // setup RPI
    calc_RPI(m_enc_advdata+14);
    size_t adv_len = 14+16;
    advData.set(m_enc_advdata, adv_len);
    sprintf_buffer(m_enc_advdata, adv_len);
    Log.info(m_tx_buffer);
}

void calc_RPI(uint8_t *data) {
    for(uint8_t i=0; i<16; i++) {
      data[i] = i;
    }  
}


// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  gap_params_init();
  BLE.setScanTimeout(100);
  advertising_init();
  BLE.advertise(&advData);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  // Scan for devices
  if( (millis() > lastSeen + RE_CHECK_MS) ){
      BLE.scan(scanResultCallback, NULL);
  }
  Log.info("isadvertising: %d", BLE.advertising());
  BleAdvertisingData tempdata;
  Log.info("advertise size: %d ", BLE.getAdvertisingData(&tempdata));
}