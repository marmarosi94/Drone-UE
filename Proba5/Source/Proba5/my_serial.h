// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#ifdef _WIN32
#include <windows.h> // Required for Serial Port API
#endif 
#include <iostream>                                                                     //std::cin, std::cout
#include <fstream>      

/**
 * 
 */

class PROBA5_API my_serial //: public AActor
{
public:
	my_serial();

	void my_serial_open();
	int32 my_serial_read(char* szBuff, int buffSize);
	void my_serial_close();
	bool isOpen() const { return is_port_open; } // Helper function
private:
	HANDLE hSerial;  // Move hSerial to a class variable
	bool is_port_open = false;
};
