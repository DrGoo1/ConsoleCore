#include "../../modules/hardware/AnalogCaptureSession.h"
#include "../../modules/hardware/MeasurementPass.h"
#include <iostream>

int main()
{
    HardwareDevice console;
    console.manufacturer = "Example";
    console.model = "British Class-A";
    console.type = HardwareType::Console;

    HardwareDevice converter;
    converter.manufacturer = "Apogee";
    converter.model = "AD-16X";
    converter.type = HardwareType::Converter;

    PatchChainDefinition patch;
    patch.chainName = "Console Capture Path";
    patch.path.push_back({"DA Output 1", "Console Input"});
    patch.path.push_back({"Console Output", "AD Input 1"});

    ConverterCalibrationProfile cal;
    cal.converterName = "Apollo X8";
    cal.sampleRate = 48000;

    AnalogCaptureSession session;
    session.sessionName = "Console Measurement Session";
    session.devices = {console, converter};
    session.patchChain = patch;
    session.converterProfile = cal;

    MeasurementPass pass;
    pass.type = MeasurementType::Sweep;
    pass.levelDbfs = -18.0f;

    std::cout << "ConsoleCore Hardware Capture CLI\n";
    std::cout << "Session: " << session.sessionName << "\n";
    std::cout << "Device Count: " << session.devices.size() << "\n";
    std::cout << "Patch Points: " << session.patchChain.path.size() << "\n";
    std::cout << "Measurement Level: " << pass.levelDbfs << " dBFS\n";

    return 0;
}