#include <cstdio>
#include <thread>
#include <chrono>
#include "fx3devifce.h"

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

// include after fx3devifce.h
#include "fx3tuner.h"


static const uint32_t ADD_FW_LOAD_PAUSE_MS      = 20;   // Pause used for loading additional fw (in ms)

using namespace std;

FX3DevIfce::FX3DevIfce() :  data_handler( NULL ), m_ref_count(1)
{
}

int FX3DevIfce::addRef()
{
    //lock_guard<System::Threading::Mutex> Lock(m_mutex);
    

    return m_ref_count++;
}
void FX3DevIfce::Release() {

    mlock(m_mutex);

    if(--m_ref_count == 0) {
        //m_mutex->ReleaseMutex();
        unmlock(m_mutex);
        delete this;
    }
    else
        unmlock(m_mutex);
}

void FX3DevIfce::changeHandler(DeviceDataHandlerIfce* handler)
{
    //lock_guard<mutex> Lock(m_mutex);

    data_handler = handler;
}

fx3_dev_err_t FX3DevIfce::getReceiverRegValue(unsigned char addr, unsigned char& value)
{
    return read8bitSPI(addr, &value);
}

fx3_dev_err_t FX3DevIfce::putReceiverRegValue(unsigned char addr, unsigned char value)
{
    return send8bitSPI(addr, value);
}


fx3_dev_err_t FX3DevIfce::resetFx3Chip() {
    return ctrlToDevice( fx3cmd::CYPRESS_RESET );
}

void FX3DevIfce::pre_init_fx3() {
    writeGPIO(NT1065EN,  0);
    writeGPIO(NT1065AOK, 0);
    writeGPIO(VCTCXOEN,  0);
    //writeGPIO(ANTLNAEN,  0);
    //writeGPIO(ANTFEEDEN, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    writeGPIO(VCTCXOEN,  1);
    writeGPIO(NT1065EN,  1);

    //writeGPIO(ANTLNAEN,  1);
    //writeGPIO(ANTFEEDEN, 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void FX3DevIfce::init_ntlab_default() {

    Fx3Tuner tuner( this );

    unsigned char reg = 0;

    //  SetSingleLO
    tuner.SetTCXO_LO( Fx3Tuner::TCXO_sel_10000kHz, Fx3Tuner::LO_src_A );

    // SetPLLTune
    tuner.SetPLL( 0, Fx3Tuner::BandL1, 1 );
    tuner.SetPLL( 1, Fx3Tuner::BandL1, 1 );

    tuner.SetFreq(0, 1590.0e6 );
    tuner.SetFreq(1, 1590.0e6 );

    //AOK
    readNtReg(0x07);

    tuner.ConfigureClockLVDS( Fx3Tuner::CLK_src_PLLB, 15, Fx3Tuner::CLK_LVDS_ampl_570mV, Fx3Tuner::CLK_LVDS_outDC_2400mV );

    // NT1065_SetSideBand( chan, side )
    int chan = 1;
    int side = 1;
    read8bitSPI(13+chan*7, &reg);
    send8bitSPI((reg&0xFD)|((side&1)<<1), 13+chan*7);

    chan = 2;
    side = 1;
    read8bitSPI(13+chan*7, &reg);
    send8bitSPI((reg&0xFD)|((side&1)<<1), 13+chan*7);

    chan = 3;
    side = 1;
    read8bitSPI(13+chan*7, &reg);
    send8bitSPI((reg&0xFD)|((side&1)<<1), 13+chan*7);

    //AOK
    readNtReg(0x07);

    // SetOutAnalog_IFMGC
    send8bitSPI(0x22, 15);
    send8bitSPI(0x22, 22);
    send8bitSPI(0x22, 29);
    send8bitSPI(0x22, 36);

//    // setRFGain
//    chan = 0;
//    int gain = 11;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0x3)|(((gain-11)&0x0F)<<4), 17+chan*7);

//    chan = 1;
//    gain = 11;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0x3)|(((gain-11)&0x0F)<<4), 17+chan*7);

//    chan = 2;
//    gain = 11;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0x3)|(((gain-11)&0x0F)<<4), 17+chan*7);

//    chan = 3;
//    gain = 11;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0x3)|(((gain-11)&0x0F)<<4), 17+chan*7);

//    // setIfGain
//    chan = 0;
//    int gain_code = 10;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0xFC)|(((gain_code>>3)&0x03)), 17+chan*7);
//    send16bitSPI( (gain_code&0x07)<<5, 18+chan*7);

//    chan = 1;
//    gain_code = 10;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0xFC)|(((gain_code>>3)&0x03)), 17+chan*7);
//    send16bitSPI( (gain_code&0x07)<<5, 18+chan*7);

//    chan = 2;
//    gain_code = 10;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0xFC)|(((gain_code>>3)&0x03)), 17+chan*7);
//    send16bitSPI( (gain_code&0x07)<<5, 18+chan*7);

//    chan = 3;
//    gain_code = 10;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0xFC)|(((gain_code>>3)&0x03)), 17+chan*7);
//    send16bitSPI( (gain_code&0x07)<<5, 18+chan*7);


    // SetLPFCalStart
    send8bitSPI(1, 4);
    std::this_thread::sleep_for( std::chrono::milliseconds(50) );

    // SetOutADC_IFMGC
    send8bitSPI(0x23, 15);
    send8bitSPI(0x0B, 19);
    send8bitSPI(0x23, 22);
    send8bitSPI(0x0B, 26);
    send8bitSPI(0x23, 29);
    send8bitSPI(0x0B, 33);
    send8bitSPI(0x23, 36);
    send8bitSPI(0x0B, 40);
}

uint32_t FX3DevIfce::GetNt1065ChipID() {

    fprintf( stderr, "GetNt1065ChipID\n" );
    unsigned char reg0 = 0;
    fx3_dev_err_t res = read8bitSPI(0x00, &reg0);
    fprintf( stderr, "read8bitSPI(0x00) ret code %d\n", res );

    unsigned char reg1 = 0;
    res = read8bitSPI(0x01, &reg1);
    fprintf( stderr, "read8bitSPI(0x01) ret code %d\n", res );

    uint32_t id = (unsigned int)reg0<<21 | ((unsigned int)reg1&0xF8)<<13 | reg1&0x07;

    fprintf( stderr, "ChipID: %08X\n", id );
    return id;
}

void print_bits( uint32_t val, int bits_count = 8 ) {
    for ( int i = bits_count - 1; i >=0; i-- ) {
        if ( val & ( 1 << i ) ) {
            fprintf( stderr, "  1" );
        } else {
            fprintf( stderr, "  0" );
        }
    }
    fprintf( stderr, "\n" );

    for ( int i = bits_count - 1; i >=0; i-- ) {
        fprintf( stderr, "%3d", i );
    }
    fprintf( stderr, "\n" );
}

void FX3DevIfce::readNtReg(uint32_t reg) {
    fx3_dev_err_t res = FX3_ERR_OK;
    unsigned char val = 0x00;
    res = read8bitSPI(reg, &val);
    fprintf( stderr, "Reg%d (0x%02X), val = 0x%08X\n", reg, reg, val );
    print_bits(val);
}

void FX3DevIfce::readFwVersion()
{
    fx3_dev_err_t res = ctrlFromDevice( fx3cmd::GET_VERSION, 0, 1, &fwDescription, sizeof(fwDescription) );
    if ( res == FX3_ERR_OK ) {
        fprintf( stderr, "\n---------------------------------"
                         "\nFirmware VERSION: %08X\n", fwDescription.version );
    } else {
        fprintf( stderr, "__error__ FX3DevIfce::readFwVersion() FAILED\n" );
    }
}


void FX3DevIfce::writeGPIO(uint32_t gpio, uint32_t value) {
#if 0
    uint32_t ans[4];
    fx3_dev_err_t res = ctrlFromDevice( fx3cmd::WRITE_GPIO, value, gpio, ans, 16 );

    if ( res == FX3_ERR_OK ) {
        if ( ans[0] != 0 ) {
            fprintf(stderr, "writeGPIO( %d, %d ) ERROR CyU3PGpioSetValue returned 0x%02X\n", gpio, value, ans[0] );
        } else {
            fprintf( stderr, "writeGPIO( %d, %d ) - ok\n", gpio, value );
        }
    } else {
        fprintf(stderr, "writeGPIO( %d, %d ) FAILED\n", gpio, value );
    }
#endif
}

void FX3DevIfce::readGPIO(uint32_t gpio, uint32_t *value) {
#if 0
    uint32_t ans[4];
    fx3_dev_err_t res = ctrlFromDevice( fx3cmd::READ_GPIO, 0, gpio, ans, 16 );

    if ( res == FX3_ERR_OK ) {
        if ( ans[0] != 0 ) {
            fprintf(stderr, "readGPIO( %d ) ERROR CyU3PGpioGetValue returned 0x%02X\n", gpio, ans[0] );
        } else {
            fprintf(stderr, "readGPIO( %d ) have %d value \n", gpio, ans[1] );
            *value = ans[1];
        }
    } else {
        fprintf(stderr, "readGPIO( %d ) FAILED\n", gpio );
    }
#endif
}

void FX3DevIfce::startGpif() {
#if 0
    fprintf( stderr, "startGpif()\n" );
    ctrlFromDevice( fx3cmd::START );
#endif
}

fx3_dev_err_t FX3DevIfce::send8bitSPI(uint8_t data, uint8_t addr)
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::read8bitSPI(uint8_t addr, uint8_t* data)
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::sendECP5(uint8_t* buf, long len)
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::recvECP5(uint8_t* buf, long len)
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::resetECP5()
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::switchoffECP5()
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::checkECP5()
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::csonECP5()
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::csoffECP5()
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::setDAC(unsigned int data)
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::device_start()
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::device_stop()
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::device_reset()
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::reset_nt1065()
{
    return FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevIfce::load1065Ctrlfile(const char* fwFileName, int lastaddr)
{
    fx3_dev_err_t retCode = FX3_ERR_OK;
    char line[128];

    FILE* pFile = fopen( fwFileName, "r" );
    if(!pFile) {
        return FX3_ERR_FIRMWARE_FILE_IO_ERROR;
    }

    while(fgets(line, 128, pFile) != NULL)
    {
        const std::string sline(line);
        if(sline[0] != ';')
        {
            size_t regpos = sline.find("Reg");
            if(regpos != std::string::npos)
            {
                std::string buf;
                std::stringstream ss(sline); // Insert the string into a stream
                std::vector<std::string> tokens;
                while(ss >> buf)
                    tokens.push_back(buf);
                if(tokens.size() == 2) // addr & value
                {
                    int addr = std::stoi(tokens[0].erase(0,3), nullptr, 10);
                    int value = std::stoi(tokens.at(1), nullptr, 16);
                    std::this_thread::sleep_for( std::chrono::milliseconds((uint64_t)ADD_FW_LOAD_PAUSE_MS) );

                    retCode = send8bitSPI(addr, value);
                    if(retCode != FX3_ERR_OK || addr == lastaddr)
                        break;
                }
            }
        }
    }

    fclose(pFile);

    return retCode;
}


