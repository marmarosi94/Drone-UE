// Fill out your copyright notice in the Description page of Project Settings.


#include "my_serial.h"


my_serial::my_serial()
{
	hSerial = INVALID_HANDLE_VALUE;
}


void my_serial::my_serial_open() {
    FString szPort = TEXT("COM3");  // Define COM port

    hSerial = CreateFile(*szPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            UE_LOG(LogTemp, Error, TEXT("Serial port COM3 not found!"));         
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to open serial port!"));
        }
        return;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get serial port state!"));
        return;
    }

    dcbSerialParams.BaudRate = CBR_460800;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to set serial port state!"));
        return;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to set serial port timeouts!"));
    }
    is_port_open = 1;
    UE_LOG(LogTemp, Log, TEXT("Serial Port COM3 Opened Successfully!"));
}

void my_serial::my_serial_close() {
    if (hSerial != INVALID_HANDLE_VALUE)
    {
        if (!CloseHandle(hSerial))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to close serial port!"));
        }
        hSerial = INVALID_HANDLE_VALUE;
        is_port_open = 0;
        UE_LOG(LogTemp, Log, TEXT("Serial Port Closed Successfully!"));
    }
}

int32 my_serial::my_serial_read(char* szBuff, int buffSize)
{
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        UE_LOG(LogTemp, Error, TEXT("Serial port not open!"));
        return 0;
    }

    DWORD dwBytesRead = 0;
    if (!ReadFile(hSerial, szBuff, buffSize, &dwBytesRead, NULL))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read from serial port!"));
        return 0;
    }

    // Null-terminate safely
    if (dwBytesRead < static_cast<DWORD>(buffSize))
        szBuff[dwBytesRead] = '\0';
    else
        szBuff[buffSize - 1] = '\0';

    return static_cast<int32>(dwBytesRead);
}
