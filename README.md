# PSOC&trade; 6 MCU: MCUboot-based bootloader with rollback to factory app

This code example implements a bootloader based on [MCUboot](https://mcu-tools.github.io/mcuboot/) to demonstrate 'rollback' to a good known image ("factory_app_cm4") in case of unrecoverable error conditions in the current application.

In this example, the bootloader loads the factory application from a known location in the external memory by directly copying it into the primary slot in the internal flash, based on user inputs during a boot. The factory app can then perform the over-the-air (OTA) upgrade to download an image over Wi-Fi and place it to the secondary slot of the MCUboot.

This code example includes the following applications:

- **bootloader_cm0p:** Bootloader is a tiny application based on [MCUboot](https://mcu-tools.github.io/mcuboot/). This is the first application which is used to start on every reset and runs entirely on the CM0+ CPU. It is responsible for validating and authenticating the firmware images in the primary and secondary slots, performing necessary upgrades, and booting the CM4 CPU

   Bootloader determines the application to run on the CM4 CPU (*blinky_cm4* or *factory_app_cm4*) depending on the state of the primary slot and user events. If you are new to MCUboot, you can try the [MCUboot-based basic bootloader](https://github.com/Infineon/mtb-example-psoc6-mcuboot-basic) example to first understand the basics

- **blinky_cm4:** This application is designed to run on the CM4 CPU. At present, this is a tiny application that blinks the LED at different rates based on build-time configurations. On successful build, a binary file is generated, which is used to demonstrate OTA firmware upgrades

- **factory_app_cm4:** The factory application is a 'golden image' that *bootloader_cm0p* can always trust and fall back on. It is built to run from the internal flash on CM4. During build, this firmware image is built to be placed in the external flash. The bootloader application transfers it to the primary slot for execution. See [Design and implementation](#design-and-implementation)


## Requirements

- [ModusToolbox&trade;](https://www.infineon.com/modustoolbox) v3.4 or later (tested with v3.4)
- Board support package (BSP) minimum required version: 4.0.0
- Programming language: C
- [Mosquitto (MQTT) broker](https://mosquitto.org/download/)
- Other tools: Python v3.8.10 or later
- Associated parts: All [PSOC&trade; 6 MCU](https://www.infineon.com/cms/en/product/microcontroller/32-bit-psoc-arm-cortex-microcontroller/psoc-6-32-bit-arm-cortex-m4-mcu/) parts
- Associated libraries:
  - [ota-update](https://github.com/Infineon/ota-update)
  - [MCUboot](https://github.com/mcu-tools/mcuboot/tree/v1.9.1-cypress)


## Supported toolchains (make variable 'TOOLCHAIN')

- GNU Arm&reg; Embedded Compiler v11.3.1 (`GCC_ARM`) – Default value of `TOOLCHAIN`


## Supported kits (make variable 'TARGET')

This example requires PSOC&trade; 6 MCU devices with at least 2 MB flash and 1 MB SRAM, and therefore, supports only the following kits:

- [PSOC&trade; 62S2 Wi-Fi Bluetooth&reg; Prototyping Kit](https://www.infineon.com/CY8CPROTO-062S2-43439) (`CY8CPROTO-062S2-43439`) – Default value of `TARGET`
- [PSOC&trade; 6 Wi-Fi Bluetooth&reg; Prototyping Kit](https://www.infineon.com/CY8CPROTO-062-4343W) (`CY8CPROTO-062-4343W`)
- [PSOC&trade; 62S2 Wi-Fi Bluetooth&reg; Pioneer Kit](https://www.infineon.com/cms/en/product/evaluation-boards/cy8ckit-062s2-43012/) (`CY8CKIT-062S2-43012`)
- [PSOC&trade; 62S2 Evaluation Kit](https://www.infineon.com/cms/en/product/evaluation-boards/cy8ceval-062s2) (`CY8CEVAL-062S2-LAI-4373M2`, `CY8CEVAL-062S2-LAI-43439M2`, `CY8CEVAL-062S2-MUR-43439M2`, `CY8CEVAL-062S2-MUR-4373EM2`, `CY8CEVAL-062S2-MUR-4373M2`)



## Hardware setup

This example uses the board's default configuration. See the kit user guide to ensure that the board is configured correctly.

> **Note:**  ModusToolbox&trade; requires KitProg3. Before using this code example, make sure that the board is upgraded to KitProg3. The tool and instructions are available in the [Firmware Loader](https://github.com/Infineon/Firmware-loader) GitHub repository. If you do not upgrade the board, you will see an error such as "unable to find CMSIS-DAP device" or "KitProg firmware is out of date".


## Software setup

See the [ModusToolbox&trade; tools package installation guide](https://www.infineon.com/ModusToolboxInstallguide) for information about installing and configuring the tools package.

- Install a terminal emulator if you do not have one. Instructions in this document use [Tera Term](https://teratermproject.github.io/index-en.html)

- This example implements a generic MQTT client that can connect to various MQTT brokers. In this code example, the instructions to set up and run the MQTT client have been provided for the AWS IoT and local Mosquitto MQTT brokers for reference. See section [Setting up the MQTT broker](#setting-up-the-mqtt-broker) for more details

- Install the Python interpreter and add it to the top of the system path in environmental variables. This code example is tested with [Python v3.8.10](https://www.python.org/downloads/release/python-3810/)


## Using the code example


### Create the project

The ModusToolbox&trade; tools package provides the Project Creator as both a GUI tool and a command line tool.

<details><summary><b>Use Project Creator GUI</b></summary>

1. Open the Project Creator GUI tool

   There are several ways to do this, including launching it from the dashboard or from inside the Eclipse IDE. For more details, see the [Project Creator user guide](https://www.infineon.com/ModusToolboxProjectCreator) (locally available at *{ModusToolbox&trade; install directory}/tools_{version}/project-creator/docs/project-creator.pdf*)

2. On the **Choose Board Support Package (BSP)** page, select a kit supported by this code example. See [Supported kits](#supported-kits-make-variable-target)

   > **Note:** To use this code example for a kit not listed here, you may need to update the source files. If the kit does not have the required resources, the application may not work

3. On the **Select Application** page:

   a. Select the **Applications(s) Root Path** and the **Target IDE**

      > **Note:** Depending on how you open the Project Creator tool, these fields may be pre-selected for you

   b. Select this code example from the list by enabling its check box

      > **Note:** You can narrow the list of displayed examples by typing in the filter box

   c. (Optional) Change the suggested **New Application Name** and **New BSP Name**

   d. Click **Create** to complete the application creation process

</details>


<details><summary><b>Use Project Creator CLI</b></summary>

The 'project-creator-cli' tool can be used to create applications from a CLI terminal or from within batch files or shell scripts. This tool is available in the *{ModusToolbox&trade; install directory}/tools_{version}/project-creator/* directory.

Use a CLI terminal to invoke the 'project-creator-cli' tool. On Windows, use the command-line 'modus-shell' program provided in the ModusToolbox&trade; installation instead of a standard Windows command-line application. This shell provides access to all ModusToolbox&trade; tools. You can access it by typing "modus-shell" in the search box in the Windows menu. In Linux and macOS, you can use any terminal application.

The following example clones the "[MCUboot-based bootloader with rollback](https://github.com/Infineon/mtb-example-wifi-mcuboot-rollback)" application with the desired name "WiFiMcubootRollback" configured for the *CY8CPROTO-062-4343W* BSP into the specified working directory, *C:/mtb_projects*:

   ```
   project-creator-cli --board-id CY8CPROTO-062-4343W --app-id mtb-example-wifi-mcuboot-rollback --user-app-name WiFiMcubootRollback --target-dir "C:/mtb_projects"
   ```

The 'project-creator-cli' tool has the following arguments:

Argument | Description | Required/optional
---------|-------------|-----------
`--board-id` | Defined in the <id> field of the [BSP](https://github.com/Infineon?q=bsp-manifest&type=&language=&sort=) manifest | Required
`--app-id`   | Defined in the <id> field of the [CE](https://github.com/Infineon?q=ce-manifest&type=&language=&sort=) manifest | Required
`--target-dir`| Specify the directory in which the application is to be created if you prefer not to use the default current working directory | Optional
`--user-app-name`| Specify the name of the application if you prefer to have a name other than the example's default name | Optional

<br>

> **Note:** The project-creator-cli tool uses the `git clone` and `make getlibs` commands to fetch the repository and import the required libraries. For details, see the "Project creator tools" section of the [ModusToolbox&trade; tools package user guide](https://www.infineon.com/ModusToolboxUserGuide) (locally available at {ModusToolbox&trade; install directory}/docs_{version}/mtb_user_guide.pdf).

</details>


### Open the project

After the project has been created, you can open it in your preferred development environment.


<details><summary><b>Eclipse IDE</b></summary>

If you opened the Project Creator tool from the included Eclipse IDE, the project will open in Eclipse automatically.

For more details, see the [Eclipse IDE for ModusToolbox&trade; user guide](https://www.infineon.com/MTBEclipseIDEUserGuide) (locally available at *{ModusToolbox&trade; install directory}/docs_{version}/mt_ide_user_guide.pdf*).

</details>


<details><summary><b>Visual Studio (VS) Code</b></summary>

Launch VS Code manually, and then open the generated *{project-name}.code-workspace* file located in the project directory.

For more details, see the [Visual Studio Code for ModusToolbox&trade; user guide](https://www.infineon.com/MTBVSCodeUserGuide) (locally available at *{ModusToolbox&trade; install directory}/docs_{version}/mt_vscode_user_guide.pdf*).

</details>


<details><summary><b>Arm&reg; Keil&reg; µVision&reg;</b></summary>

Double-click the generated *{project-name}.cprj* file to launch the Keil&reg; µVision&reg; IDE.

For more details, see the [Arm&reg; Keil&reg; µVision&reg; for ModusToolbox&trade; user guide](https://www.infineon.com/MTBuVisionUserGuide) (locally available at *{ModusToolbox&trade; install directory}/docs_{version}/mt_uvision_user_guide.pdf*).

</details>


<details><summary><b>IAR Embedded Workbench</b></summary>

Open IAR Embedded Workbench manually, and create a new project. Then select the generated *{project-name}.ipcf* file located in the project directory.

For more details, see the [IAR Embedded Workbench for ModusToolbox&trade; user guide](https://www.infineon.com/MTBIARUserGuide) (locally available at *{ModusToolbox&trade; install directory}/docs_{version}/mt_iar_user_guide.pdf*).

</details>


<details><summary><b>Command line</b></summary>

If you prefer to use the CLI, open the appropriate terminal, and navigate to the project directory. On Windows, use the command-line 'modus-shell' program; on Linux and macOS, you can use any terminal application. From there, you can run various `make` commands.

For more details, see the [ModusToolbox&trade; tools package user guide](https://www.infineon.com/ModusToolboxUserGuide) (locally available at *{ModusToolbox&trade; install directory}/docs_{version}/mtb_user_guide.pdf*).

</details>


## Operation

This code example expects you to be familiar with MCUboot and its concepts. See [MCUboot](https://github.com/mcu-tools/mcuboot) to learn more.

This example bundles three applications: the bootloader app run by CM0+, the factory app, and the blinky app run by CM4. You need to build and program the applications in the following order. Do not start building the applications yet. Follow the [Step-by-step instructions](#step-by-step-instructions).

1. **Build and program the bootloader app** - On the next reset, CM0+ runs the bootloader app and prints a message, that no valid image has been found

2. **Connect publisher client to MQTT broker** - By default, the *publisher.py* script is written to connect with the local MQTT broker in non-TLS mode using an IP address. Follow the instructions from [Setting up the MQTT publisher script](#setting-up-the-mqtt-publisher-script) to connect the publisher client to the MQTT broker

3. **Build and program the factory app to the external flash** - By default, the *factory_app_cm4* application is built for the external flash. Follow the bootloader instructions on the console to initiate a rollback using the user button

4. **Build the blinky app in UPGRADE mode (default)** - A binary will be generated on successful build, which will be used for OTA upgrade demonstration

5. **Perform OTA upgrade to boot to blinky app** - Edit the *ota_update.json* file to modify the value of `Version` to `2.0.0`. The factory app now finds the updated job document, downloads the new image, and places it in the secondary slot. Once the download is complete, a soft reset is issued. The MCUboot bootloader then starts the image upgrade process


## Setting up the MQTT broker

The root directory of the factory application is referred to as *\<factory_app_cm4>* in this code example.


### Using AWS IoT Core

1. Set up the MQTT device (also known as a *Thing*) in the AWS IoT Core as described in the [Getting started with AWS IoT tutorial](https://docs.aws.amazon.com/iot/latest/developerguide/iot-gs.html)

   > **Note:** While setting up your device, ensure that the policy associated with this device permits all MQTT operations (*iot:Connect*, *iot:Publish*, *iot:Receive*, and *iot:Subscribe*) for the resource used by this device. For testing purposes, it is recommended to have the following policy document which allows all *MQTT Policy Actions* on all *Amazon Resource Names (ARNs)*

   ```
   {
        "Version": "2012-10-17",
        "Statement": [
            {
                "Effect": "Allow",
                "Action": "iot:*",
                "Resource": "*"
            }
        ]
   }
   ```

2. Download the following certificates and keys that are created and activated in the previous step:
   - A certificate for the AWS IoT Thing: *xxxxxxxxxx.aws-client-certificate.crt*
   - A private key: *xxxxxxxxxx.aws-private.key* or *xxxxxxxxxx.aws-private.pem*
   - Root CA "RSA 2048 bit key: Amazon Root CA 1" for AWS IoT from [CA certificates for server authentication](https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html#server-authentication-certs) - *xxxx.AmazonRootCA1.crt*

3. Copy the certificates and key, and paste it in the *\<WIFI_MCUBOOT_ROLLBACK>/scripts* folder

4. Rename the following file names in the *\<WIFI_MCUBOOT_ROLLBACK>/scripts* folder
   - *xxxx.AmazonRootCA1.crt* to *aws_ca.crt*
   - *xxxxxxxxxx.aws-client-certificate.crt* to *aws_client.crt*
   - *xxxxxxxxxx.aws-private.key* to *aws_private.key*


### Mosquitto local broker

This code example uses the locally installable Mosquitto MQTT broker that runs on your computer as the default broker. You can also use one of the other public MQTT brokers listed at [public_brokers](https://github.com/mqtt/mqtt.github.io/wiki/public_brokers).

1. Download the executable setup from the [Mosquitto](https://mosquitto.org/download/) website

2. Run the setup to install the software. During installation, uncheck the **Service** component. Also, note down the installation directory

3. Once the installation is complete, add the installation directory to the system `PATH`

4. Open a CLI terminal

   On Linux and macOS, you can use any terminal application. On Windows, open the 'modus-shell' app from the Start menu

5. Navigate to the *<WIFI_MCUBOOT_ROLLBACK>/scripts* folder

6. Execute the following command to generate self-signed SSL certificates and keys. On Linux and macOS, you can get your device local IP address by running the `ifconfig` command on any terminal application. On Windows, run the `ipconfig` command on a command prompt


   ```
   sh generate_ssl_cert.sh <local-ip-address-of-your-pc>
   ```

   For example:
   ```
   sh generate_ssl_cert.sh 192.168.0.10
   ```

   This step will generate the following files in the same *<WIFI_MCUBOOT_ROLLBACK>/scripts/* directory:

   - *mosquitto_ca.crt:* Root CA certificate
   - *mosquitto_ca.key:* Root CA private key
   - *mosquitto_server.crt:* Server certificate
   - *mosquitto_server.key:* Server private key
   - *mosquitto_client.crt:* Client certificate
   - *mosquitto_client.key:* Client private key

7. The *<WIFI_MCUBOOT_ROLLBACK>/scripts/mosquitto.conf* file is pre-configured for starting the Mosquitto server for this code example. You can edit the file if you wish to make other changes to the broker settings

8. Start the Mosquitto server:

   - **Using the code example in TLS mode (default):**

      1. Execute the following command:

         ```
         mosquitto -v -c mosquitto.conf
         ```

   - **Using the code example in non-TLS mode:**

      1. Edit the *<WIFI_MCUBOOT_ROLLBACK>/scripts/mosquitto.conf* file and change the value of the `require_certificate` parameter to `false`

      2. Execute the following command:

         ```
         mosquitto -v -c mosquitto.conf
         ```


## Setting up the MQTT publisher script

1. Open a CLI terminal

   On Linux and macOS, you can use any terminal application. On Windows, open the 'modus-shell' app from the Start menu

2. Navigate to the *<WIFI_MCUBOOT_ROLLBACK>/scripts* folder

3. Edit the *<WIFI_MCUBOOT_ROLLBACK>/scripts/publisher.py* file to configure your MQTT publisher (MQTT server)

   1. Modify the value of the `BOARD` variable to your selected `TARGET` in the following format

      ```
      if TARGET=APP_CY8CPROTO-062S2-43439, then BOARD = "APP_CY8CPROTO_062S2_43439"
      ```
      
      Example:
      ```
      BOARD = "APP_CY8CPROTO_062S2_43439"
      ```
      
      > **Note:** Ensure to change the `-` to `_` in the `BOARD` variable value after copied from the `TARGET` variable

   2. Modify the value of the `AMAZON_BROKER_ADDRESS` variable to your custom endpoint on the **Settings** page of the AWS IoT console. This has the format `ABCDEFG1234567.iot.<region>.amazonaws.com`
      
      > **Note:** If you are using the local MQTT broker (e.g., Mosquitto broker), modify the value of `MOSQUITTO_BROKER_LOCAL_ADDRESS` to the local IP address of your MQTT broker

   3. Ensure the value of the `BROKER_ADDRESS` variable is `AMAZON_BROKER_ADDRESS`
      
      > **Note:** If you are using the local MQTT broker (e.g., Mosquitto broker), modify the value of `BROKER_ADDRESS` to `MOSQUITTO_BROKER_LOCAL_ADDRESS`

   4. Ensure that the value of the `TLS_ENABLED` variable is `True`

   5. Ensure that the value of the `BROKER_PORT` variable is `8883`
       
      > **Note:** If you are using the local MQTT broker (e.g., Mosquitto broker), ensure that the value of `BROKER_PORT` variable is `8884`. Currently in the *publisher.py* file conditional `if-else` block is used to automatically select a `BROKER_PORT` value based on the selected MQTT broker

4. Ensure that the certificate and key file names in the *scripts* folder and following variables value in the *<WIFI_MCUBOOT_ROLLBACK>/scripts/publisher.py* file are the same
      - `ca_certs` = "aws_ca.crt"
      - `certfile` = "aws_client.crt"
      - `keyfile`  = "aws_private.key"

      These variables are located in the `AMAZON BROKER` section at the last line in the *\<WIFI_MCUBOOT_ROLLBACK>/scripts/publisher.py* file
      
      > **Note:** If you are using the local MQTT broker (e.g., Mosquitto broker), ensure that the certificate and key file names in the *\<WIFI_MCUBOOT_ROLLBACK>/scripts* folder and these variables value in the *\<WIFI_MCUBOOT_ROLLBACK>/scripts/publisher.py* file under the `MOSQUITTO_BROKER_LOCAL_ADDRESS` section are the same. Currently in the *publisher.py* file, the conditional `if-else` block is used to automatically select the default certificate and key file names based on the selected MQTT broker

5. Run the *publisher.py* Python script

   The script takes arguments such as the kit name, broker URL, and file path. For details on the supported arguments and their usage, execute the following command

   > **Note:** For Linux and macOS platforms, use `python3` instead of `python` in the following command:

   ```
   python publisher.py --help
   ```

   To start the publisher script for the default settings of this example, execute the following command:

   ```
   python publisher.py tls
   ```
   > **Note:** The publisher script does not support non-TLS mode in the local MQTT server. You can always use the publisher script in TLS mode


### Step-by-step instructions

The *bootloader_cm0p* app design is based on MCUboot, which uses the [imgtool](https://pypi.org/project/imgtool/) Python module for image signing and key management

1. Connect the board to your PC using the provided USB cable through the KitProg3 USB connector. Open a terminal program and select the KitProg3 COM port. Set the serial port parameters to 8N1 and 115200 baud

2. The bootloader, factory app, and blinky app must have the same understanding of the memory layout. The memory layout is defined through JSON files. The *flashmap* folder provides a set of predefined JSON files that can be readily used. All applications must use the same JSON file

   **Table 1. Supported JSON files**

   Target      | Supported JSON files
   ----------- |----------------------------------
   CY8CPROTO-062S2-43439 <br>CY8CPROTO-062-4343W <br> CY8CKIT-062S2-43012  <br>CY8CEVAL-062S2-LAI-4373M2<br>CY8CEVAL-062S2-LAI-43439M2<br>CY8CEVAL-062S2-MUR-43439M2<br>CY8CEVAL-062S2-MUR-4373EM2<br>CY8CEVAL-062S2-MUR-4373M2<br> | Targets support the following flashmaps:  <br> *psoc62_swap_single_smif.json* (Default)<br> *psoc62_overwrite_single_smif.json*
    

   <br>

3. Modify the value of the `FLASH_MAP` variable in *user_config.mk* to the selected JSON file name from the previous step

4. Install the dependent modules for the [imgtool](https://pypi.org/project/imgtool/) Python module for image signing and key management

   MCUboot already includes this module but not the dependent modules. Do the following:

   1. Open a CLI terminal and navigate to the *\<mtb_shared>/mcuboot/\<tag>/scripts* directory

      On Windows, use the command line "modus-shell" program provided in the ModusToolbox&trade; installation instead of a standard Windows command line application. This shell provides access to all ModusToolbox&trade; tools. You can access it by typing `modus-shell` in the search box in the Windows menu

      In Linux and macOS, you can use any terminal application

   2. Run the following command to ensure that the required Python modules are installed
   
      > **Note:** For Linux and macOS platforms, use `python3` instead of `python` in the following command:
       ```
       python -m pip install -r requirements.txt
       ```
       ```
       python -m pip install paho-mqtt==1.6.1
       ```

5. Set up the MQTT broker and connect the publisher client to the broker from [Setting up the MQTT broker](#setting-up-the-mqtt-broker) and [Setting up the MQTT publisher script](#setting-up-the-mqtt-publisher-script) to connect the publisher client to the MQTT broker

6. Edit the *\<Factory App>/configs/ota_app_config.h* file to configure your *factory_app_cm4* image:

   1. Modify the connection configuration, such as `WIFI_SSID`, `WIFI_PASSWORD`, and `WIFI_SECURITY` macros to match the settings of your Wi-Fi network

       > **Note:** If you are using the local MQTT broker (e.g., Mosquitto broker), ensure that the device running the MQTT local broker and the kit are connected to the same network

   2. Modify the value of the `MQTT_BROKER_URL` macro to your custom endpoint on the **Settings** page of the AWS IoT console. This has the format `abcdefg1234567.iot.<region>.amazonaws.com`

       > **Note:** If you are using the local MQTT broker (e.g., Mosquitto broker), modify the value of `MQTT_BROKER_URL` to the local IP address of your MQTT broker

   3. Ensure that the value of the `MQTT_SERVER_PORT` macro is `8883`

       > **Note:** If you are using the local MQTT broker (e.g., Mosquitto broker), modify the value of `MQTT_SERVER_PORT` to `8884`. If the code example has been configured to work in non-TLS mode, set the value of `MQTT_SERVER_PORT` to `1883`

   4. By default, this code example works in TLS mode. To use the example in non-TLS mode, modify `ENABLE_TLS` to **false** and skip the next step of adding the certificate

   5. Add the certificates and key:

      1. Open a CLI terminal

          On Linux and macOS, you can use any terminal application. On Windows, from the Start menu, open **modus-shell** app

      2. Navigate the terminal to the *\<OTA_MQTT>/scripts* directory

      3. Run the *format_cert_key.py* Python script to generate the string format of the certificate and key files that can be added as a macro. Pass the name of the certificate or key with the extension as an argument to the Python script:

         > **Note:** For Linux and macOS platforms, use `python3` instead of `python` in the following command

         ```
         python format_cert_key.py <one-or-more-file-name-of-certificate-or-key-with-extension>
         ```

         Example:
         ```
         python format_cert_key.py aws_ca.crt aws_client.crt aws_private.key
         ```

         You can either convert the values to strings by running the *format_cert_key.py* scripts like shown above or you can use the [HTML utility](https://github.com/Infineon/amazon-freertos/blob/master/tools/certificate_configuration/PEMfileToCString.html) to convert the certificates and keys from PEM format to C string format. You need to clone the repository from GitHub to use the utility

      4. Copy the generated strings and add it to the `ROOT_CA_CERTIFICATE`, `CLIENT_CERTIFICATE` and `CLIENT_KEY` macros per the sample shown

7. Edit the job document (*<WIFI_MCUBOOT_ROLLBACK>/scripts/ota_update.json*):
   
   1. Modify the value of `Broker` to match the value of the `MQTT_BROKER_URL` and `MQTT_BROKER_URL` variables present in the *\<OTA_MQTT>/configs/ota_app_config.h* file

   2. Modify the value of the variable `Board` to your selected `TARGET` in the following format

      ```
      if TARGET=APP_CY8CPROTO-062S2-43439, then Board:"APP_CY8CPROTO_062S2_43439"
      ```
      Example:
      ```
      "Board":"APP_CY8CPROTO_062S2_43439",
      ```
      > **Note:** Ensure to change the `-` to `_` in the `Board` variable value while copying from the `TARGET` variable

   3. Ensure the value of the `Port` macro is `8883`
      
      > **Note:** If you are using the local MQTT broker (e.g., Mosquitto broker), modify the value of `Port` to `8884`. If the code example has been configured to work in non-TLS mode, set the value of `Port` to `1883`

8. Program the board using one of the following:

9. Build and program the bootloader application to the internal flash and the factory application to the external flash

   <details><summary><b>Using Eclipse IDE for ModusToolbox&trade;</b></summary>

      1. Select the *bootloader_cm0p* application in the Project Explorer

      2. In the **Quick Panel**, scroll down, and click **\<Application Name> Program (KitProg3_MiniProg4)**
   </details>


   <details><summary><b>Using CLI</b></summary>

   - **Bootloader application using CLI:**

   1. Go to the *bootloader_cm0p* directory and execute the `make program_proj` command to build and program the bootloader application using the default toolchain to the default target

      You can specify a target and toolchain manually using the following command:
      ```
      make program_proj TARGET=<BSP> TOOLCHAIN=<toolchain>
      ```

      Example:
      ```
      make program_proj TARGET=CY8CPROTO-062-4343W TOOLCHAIN=GCC_ARM
      ```

   - **Factory app using CLI:**

      Go to the *factory_app_cm4* directory and execute the `make program_proj` command to build and program the factory app using the default toolchain to the default target. You can specify a target and toolchain manually:
      ```
      make program_proj TARGET=<BSP> TOOLCHAIN=<toolchain>
      ```

      Example:
      ```
      make program_proj TARGET=CY8CPROTO-062-4343W TOOLCHAIN=GCC_ARM
      ```

   </details>

   After programming, the bootloader application starts automatically. Confirm that the UART terminal displays the following message:

   **Figure 1. Bootloader starting with no bootable image**

   ![](images/booting-with-no-bootable-image.png)

10. Initiate a rollback to the factory app

      At this point, both the primary and secondary slots are empty. The bootloader reports that both the slots are empty and waits for the user's action, as shown in **Figure 1**

      Press and release the user button to initiate rollback

      The bootloader detects the *factory_app_cm4* application in the external flash, copies it to the primary slot, validates it and starts the application on CM4. On successful boot, the *factory_app_cm4* application will boot to the console and start blinking the user LED at a 1 Hz rate

      **Figure 2. Rollback to factory_app_cm4 application when both primary and secondary slots are empty**

     ![](images/roll-back-when-both-slots-are-empty.png)

11. Build the blinky application in UPGRADE mode (**DO NOT** program it to the kit)

    <details><summary><b>Using Eclipse IDE for ModusToolbox&trade;</b></summary>

      Note that `IMG_TYPE` is set to `UPGRADE` by default in *blinky_cm4/Makefile*.

      1. Select the `blinky_cm4` application in the Project Explorer

      2. In the **Quick Panel**, scroll down, and click **\<Application Name> Build Application**
    
    </details>


    <details><summary><b>Using CLI</b></summary>

    From the terminal, go to the *blinky_cm4* directory and execute the `make build_proj` command. You can specify a target and toolchain manually using the following command:
    ```
    make build_proj TARGET=<BSP> TOOLCHAIN=<toolchain>
    ```
 
    For Example:
    ```
    make build_proj TARGET=CY8CPROTO-062-4343W TOOLCHAIN=GCC_ARM
    ```
    </details>

    > **Note:** The *<APPNAME>.bin* binary file generated at the end of a successful build will be used in subsequent steps for the OTA upgrade.

12. Download the blinky application to the bootloader's secondary slot (using the OTA capabilities of the factory application)

    After a successful build, edit the *scripts/ota_update.json* file to modify the value of 'version' to '2.0.0'

    The factory app starts blinking the user LED at a 1 second interval on bootup and waits for the user button event to start the OTA upgrade process. Press and release the user button to start the OTA upgrade process

    When a user button press is detected, the device establishes a connection with the designated MQTT broker (AWS is used in this example) and subscribes to a topic. When an OTA image is published to that topic, the device automatically pulls the OTA image over MQTT and saves it to the secondary slot

    **Figure 3. Factory app ready for OTA upgrade**

    ![](images/factory-img-boot.png)

    Observe the UART terminal to see the OTA image being received in chunks

    **Figure 4. Receiving a new firmware image**

    ![](images/ota-and-boot-to-user-app.png)

    Once all the chunks are received and written to the secondary slot, the factory app will reset the device. On reset, the bootloader will verify the new image in the secondary slot, copy it to the primary slot, and boot the newly downloaded blinky app

    - To confirm the swap of the upgrade image from the secondary slot into the primary slot and make it the primary image, enter `Y` in the UART terminal
 
    - To revert to the original image, enter `N`

    Confirm that the user LED toggles at a 250 millisecond interval. On every reset, the bootloader app will start the user application in the primary slot as long as it is valid


## Developer notes

Do **NOT** Write/Erase the external memory region allocated to store *factory_app_cm4* application. If the factory app is erased from the external flash, the bootloader will detect and report an error to the user on the next rollback attempt.


## Debugging

You can debug the example to step through the code.


<details><summary><b>In Eclipse IDE</b></summary>

Use the **\<Application Name> Debug (KitProg3_MiniProg4)** configuration in the **Quick Panel**. For details, see the "Program and debug" section in the [Eclipse IDE for ModusToolbox&trade; user guide](https://www.infineon.com/MTBEclipseIDEUserGuide).

</details>


<details><summary><b>In other IDEs</b></summary>

Follow the instructions in your preferred IDE.

</details>


## Design and implementation

### Overview

This example bundles three applications: the bootloader app, the blinky app, and the factory app. By default, the blinky app is built in UPGRADE mode to demonstrate the OTA upgrade process. The factory app is built to be placed in the external memory and copied to the internal flash on rollback; it executes entirely from the internal flash. It supports the [OTA](https://github.com/Infineon/ota-update) process. You can trigger the OTA upgrade as described in [Step-by-step instructions](#step-by-step-instructions).


### Bootloader implementation

The bootloader is designed based on the MCUboot repo in [GitHub](https://github.com/mcu-tools/mcuboot/tree/v1.9.1-cypress). It is customized in this example to support the rollback feature. Details of the design are provided in the following sections. The bootloader supports both overwrite and swap modes.

**Figure 5. bootloader_cm0p implementation overview**

![](images/mcuboot-design.png)


#### Rollback when both the primary and secondary slots are invalid

On bootup, the bootloader checks both primary and secondary slots and determines whether a valid image is present. If both the slots are empty or invalid, the bootloader displays a message on the console stating that there are no valid images in either of the slots.

You can press and release the user button to initiate the rollback as instructed in Step 9 (initiate rollback to the factory app) in the [Step-by-step instructions](#step-by-step-instructions) section.

#### Rollback when the primary slot is valid but the secondary slot is invalid

If the primary slot is valid and no upgradable image is present in the secondary slot, on reset, the bootloader boots to the primary slot.  Instead of booting the application, press and hold the user button during the boot until the **Rollback Initiated** message is seen on the console to initiate a rollback.

If the device has already booted to the application in the primary slot, you can initiate a rollback by pressing the user button and then initiating a reset. In both cases, you must press and hold the user button during the boot for approximately 5 seconds until you observe the **Rollback Initiated** message on the console.


#### Rollback when the secondary slot has a valid upgradable image

By design, an upgrade always gets priority over a rollback request. The bootloader will run the upgrade process first. User events are ignored during the upgrade process.

However, at the end of the upgrade process (before booting to the new image), the bootloader will check the user button status to determine if a rollback is requested. Rollback will be initiated if the user button is held pressed.

#### Recovering from a power failure during rollback

The bootloader application provides a built-in recovery mechanism from power failure. It validates the primary slot on every reset and reports the status via console messages. If the bootloader reports no valid images, the device can be restored back to its functional state by initiating a rollback. See [Rollback when both the primary and secondary slots are invalid](#rollback-when-both-the-primary-and-secondary-slots-are-invalid).

**Figure 6** shows the console messages. Messages in the highlighted text boxes indicate power failure and MCU reset while copying the *factory_app_cm4* image to the primary slot. These messages indicate that on the next boot, a rollback was initiated with a user button event.

**Figure 6. Power failure during rollback**

![](images/power-failure-recovery.png)


### Blinky app implementation

This is a tiny application that simply blinks the user LED on startup. The LED blink interval is configured based on the `IMG_TYPE` specified. By default, `IMG_TYPE` is set to `UPGRADE` to generate suitable binaries for upgrade.

The bootloader upgrade image is in an overwrite mode in this case; the LED toggles at a 250 millisecond interval. The bootloader will copy this new image from the secondary into the primary slot and the upgrade image, run from the primary slot. The user can make the upgrade image a permanent image in the primary slot by entering **Y** in the UART terminal or revert to the older image by entering **N** in the UART terminal on reset. Confirm that the user LED toggles at the 250 millisecond interval.

The image will be signed using the keys available in *keys*. It is possible to build the blinky application as a 'BOOT' image and program it to the primary slot directly (not discussed in this document).


### Factory app implementation

The factory app uses the [ota-update](https://github.com/Infineon/ota-update) middleware. It performs an OTA upgrade using the MQTT protocol. The application connects to the MQTT server and receives the OTA upgrade package, if available. The OTA upgrade image will be downloaded to the secondary slot of MCUboot in chunks. Once the complete image is downloaded, the application issues an MCU reset. On reset, the bootloader starts and handles the rest of the upgrade process.

The factory app is signed using the keys available under *keys* to ensure that the bootloader boots it safely. This process detects malicious firmware or possible corruptions early in the boot process.

**Figure 7. factory_app_cm4 implementation overview**

![](images/factory-image-bootflow.png)


### Memory layout in PSOC&trade; 6 MCU with 2 MB flash

The development kit has a 2 MB internal flash and a 64 MB external flash, which is partitioned into the following regions:

**Table 2. Flash partition into regions**

Region | Size <br> *psoc62_swap_single_smif.json* *psoc62_overwrite_single_smif.json* | Description
---------------------|-------------|--------
Bootloader partition | 96 KB | Partition is the first region in the internal flash memory that holds *bootloader_cm0p*
Primary partition    | 1792 KB | Partition where the bootable application is placed. Primary boot slot of *bootloader_cm0p*
Secondary partition  | 1792 KB | Partition where the downloaded OTA upgrade image will be placed. Secondary slot of *bootloader_cm0p*
Scratch Region           | 512 KB | Region used for swapping image. Only applicable when *swap* policy is used
RFU                  | 160 KB | Region reserved for future use
Factory app reserved | First 2 MB in external flash | Region reserved for *factory_app_cm4*. Although the maximum size of the factory app cannot exceed the primary slot size, an additional 72 KB (1 MB – 952 KB) is allocated to align with the erase sector size of the QSPI flash (S25FL512S)
User region            | 60 MB | Region for application use

<br>

**Figure 8. Memory layout**

![](images/flash-partition.png)


### Make variable default configuration

This section explains the important make variables that affect this code example's functionality. You can either update these variables directly in the Makefile or pass them along with the `make build` command.

**Table 3. Common make variables**

Variable                  | Default value        | Description
--------------------------| -------------------- |-----------------------------------------------------------------------------------
`SIGN_KEY_FILE`           | cypress-test-ec-p256 | Name of the private and public key files (the same name is used for both the keys)
`BOOTLOADER_SIZE`           | Autogenerated       | Flash size of the bootloader app run by CM0+ <br>In the linker script for the bootloader app (CM0+), the `LENGTH` of the `flash` region is set to this value<br>In the linker script for the blinky app (CM4), the `ORIGIN` of the `flash` region is offset to this value
`BOOTLOADER_APP_RAM_SIZE`   | 0x20000              | RAM size of the *bootloader_cm0p* app run by CM0+ <br>In the linker script for the *bootloader_cm0p* app (CM0+), `LENGTH` of the `ram` region is set to this value<br>In the linker script for the blinky app (CM4), `ORIGIN` of the `ram` region is offset to this value and `LENGTH` of the `ram` region is calculated based on this value
`SLOT_SIZE`                 | Autogenerated       | Size of the primary slot and secondary slot. i.e., the flash size of the blinky app run by CM4
`MCUBOOT_HEADER_SIZE`       | 0x400                | Size of the MCUboot header. Must be a multiple of 1024 (see the note below)<br>Used in the following places:<br>1. In the linker script for the blinky app (CM4), the starting address of the`.text` section is offset by the MCUboot header size from the `ORIGIN` of the `flash` region. This is to leave space for the header that will be later inserted by the *imgtool* during the post-build process <br>2. Passed to the *imgtool* utility while signing the image. The *imgtool* utility fills the space of this size with zeroes (or 0xff depending on internal or external flash) and then adds the actual header from the beginning of the image
`MAX_IMG_SECTORS`           | Autogenerated       | Maximum number of flash sectors (or rows) per image slot for which swap status is tracked in the image trailer
`MCUBOOT_IMAGE_NUMBER`      | Autogenerated       | The number of images supported in the case of multi-image bootloading
`PRIMARY_IMG_START`         | Autogenerated       | Starting address of the primary slot
`SECONDARY_IMG_START`        | Autogenerated       | Starting address of the secondary slot
`FACT_APP_SIZE`             | 0x100000(USE_EXTERNAL_FLASH=0) <br> 0x200000(USE_EXTERNAL_FLASH=1)             | Reserved size for *factory_app_cm4* in the external flash. This size must be the same as `SLOT_SIZE`. However, 1 MB is allocated to make it aligned with the 256 KB sector size of S25FL512S
`EXTERNAL_FLASH_SIZE`       | 0x4000000            | Size of the external flash memory available on the development kit. A 64 MB QSPI NOR flash [S25FL512S](https://www.infineon.com/dgdl/Infineon-S25FL512S_512_Mb_(64_MB)_3.0_V_SPI_Flash_Memory-DataSheet-v19_00-EN.pdf?fileId=8ac78c8c7d0d8da4017d0ed046ae4b53) is used on the kits supported in this CE

> **Note:** The value of `MCUBOOT_HEADER_SIZE` must be a multiple of 1024 because the CM4 image begins immediately after the MCUboot header, and it begins with the interrupt vector table. For PSOC&trade; 6 MCU, the starting address of the interrupt vector table must be 1024-byte aligned.

**Table 4. bootloader_cm0p make variables**

Variable               | Default value | Description  
---------------------- | ------------- | ------------------------------------------------------------
`USE_OVERWRITE`              | Autogenerated       | Value is '1' when scratch and status partitions are not defined in the flashmap JSON file
`USE_EXTERNAL_FLASH`         | Autogenerated       | Value is '1' when external flash is used for either the primary or secondary slot

 <br>

**Table 5. factory_app_cm4 make variables**

Variable             | Default value | Description
---------------------| ------------- | ---------------------- 
`HEADER_OFFSET`        | 0x7FE8000     | Starting address of the CM4 app or the offset at which the header of an image will begin. *Image = Header + App + TLV + Trailer* <br>New relocated address = `ORIGIN` + `HEADER_OFFSET`<br>`ORIGIN` is defined in the CM4 linker script and is usually the address next to the end of the CM0+ image. <br>See the table below for the values of `HEADER_OFFSET` for the default flash map
`USE_OVERWRITE`              | Autogenerated       | Value is '1' when scratch and status partitions are not defined in the flashmap JSON file
`USE_EXTERNAL_FLASH`         | Autogenerated       | Value is '1' when an external flash is used for either primary or secondary slot
`SIGN_KEY_FILE_PATH` | *../keys* | Path to the private key file. Used with the *imgtool* for signing the image
`APP_VERSION_MAJOR`<br>`APP_VERSION_MINOR`<br>`APP_VERSION_BUILD` | 1.0.0 if `IMG_TYPE=BOOT`<br>2.0.0 if `IMG_TYPE=UPGRADE` | Passed to the *imgtool* with the `-v` option in *MAJOR.MINOR.BUILD* format, while signing the image. Also available as macros to the application with the same names

<br>

**Table 6. blinky_cm4 make variables**

| Variable      | Default value | Description
| ------------- | ------------- | ---------------------- 
| `IMG_TYPE`      | UPGRADE       | Valid values are `BOOT` and `UPGRADE`. Default value is set to `UPGRADE` in this code example along with padding. Set it to `BOOT` if you want to flash it directly on to the primary slot instead of upgrading
| `HEADER_OFFSET` | Auto-calculated             | The starting address of the CM4 app or the offset at which the header of an image will begin. Value equal to (`SECONDARY_IMG_START` - `PRIMARY_IMG_START`)
| `USE_OVERWRITE`              | Autogenerated       | Value is '1' when scratch and status partitions are not defined in the flashmap JSON file
 `USE_EXTERNAL_FLASH`         | Autogenerated       | Value is '1' when an external flash is used for either primary or secondary slot
 `SIGN_KEY_FILE_PATH` | *../keys* | Path to the private key file. Used with the *imgtool* for signing the image
 `APP_VERSION_MAJOR`<br>`APP_VERSION_MINOR`<br>`APP_VERSION_BUILD` | 1.0.0 if `IMG_TYPE=BOOT`<br>2.0.0 if `IMG_TYPE=UPGRADE` | Passed to the *imgtool* with the `-v` option in *MAJOR.MINOR.BUILD* format, while signing the image. Also available as macros to the application with the same names

<br>


### Security

> **Note:** This example simply demonstrates the image-signing feature of MCUboot. It does not implement root of trust (RoT)-based secure services such as secure boot and secure storage (to securely store and retrieve the keys). You must ensure that adequate security measures are implemented in your end product. See the [PSOC&trade; 64 line of secured MCUs](https://www.infineon.com/cms/en/product/microcontroller/32-bit-psoc-arm-cortex-microcontroller/psoc-6-32-bit-arm-cortex-m4-mcu/psoc-64/) that have built-in advanced security features. See the [whitepaper](https://www.infineon.com/dgdlac/Infineon-Security_Comparison_Between_PSoC_64_Secure_MCU_and_PSoC_62_63_MCU-Whitepaper-v01_00-EN.pdf?fileId=8ac78c8c7d0d8da4017d0fb27691270a) that compares the security features between the PSOC&trade; 64 Secured MCUs and the PSOC&trade; 62/63 MCUs.

MCUboot checks image integrity with SHA256, and image authenticity with digital signature verification. Multiple signature algorithms are supported; this example enables ECDSA SECP256R1 (EC256) by default. MCUboot uses the Mbed TLS library for cryptography. PSOC&trade; 6 MCU supports hardware-accelerated cryptography based on the Mbed TLS library via a shim layer. The [cy-mbedtls-acceleration](https://github.com/Infineon/cy-mbedtls-acceleration) library implements this layer.

MCUboot verifies the signature of the image in the primary slot every time before booting when `MCUBOOT_VALIDATE_PRIMARY_SLOT` is defined. In addition, it verifies the signature of the image in the secondary slot before copying it to the primary slot.

This example enables image authentication by uncommenting the following lines in the *bootloader_cm0p/libs/mcuboot/boot/cypress/MCUBootApp/config/mcuboot_config/mcuboot_config.h* file:

```
#define MCUBOOT_SIGN_EC256
#define NUM_ECC_BYTES (256 / 8)
.
.
.
#define MCUBOOT_VALIDATE_PRIMARY_SLOT
```

When these options are enabled, the public key is embedded within the bootloader app. The blinky and factory apps are signed using the private key during the post-build steps. The *imgtool* Python module included in the MCUboot repository is used for signing the image.

This example includes a sample key-pair under the *keys* directory. You must not use this key-pair in your end product.  For more details on key management, see [imgtool](https://docs.mcuboot.com/imgtool.html).


### Resources and settings

**Table 7. Bootloader app**

Resource  |  Alias/object     |    Purpose
:------- | :------------    | :------------
SCB UART (PDL) |CYBSP_UART| Used for redirecting `printf` to the UART port
SMIF (PDL) | QSPIPort | Used for interfacing with the QSPI NOR flash
GPIO (HAL)    | CYBSP_USER_BTN         | User button

<br>

**Table 8. Blinky app**

Resource  |  Alias/object     |    Purpose
:-------- | :-------------    | :------------
UART (HAL)|cy_retarget_io_uart_obj| UART HAL object used by Retarget-IO for the Debug UART port
GPIO (HAL)    | CYBSP_USER_LED         | User LED

<br>

**Table 9. Factory app**

Resource  |  Alias/Object     |    Purpose
:------- | :------------    | :------------
UART (HAL)|cy_retarget_io_uart_obj| UART HAL object used by Retarget-IO for the Debug UART port
GPIO (HAL)    | CYBSP_USER_LED         | User LED
GPIO (HAL)    | CYBSP_USER_BTN         | User button

<br>


## Related resources

Resources  | Links
-----------|----------------------------------
Application notes  | [AN228571](https://www.infineon.com/AN228571) – Getting started with PSOC&trade; 6 MCU on ModusToolbox&trade; <br>  [AN215656](https://www.infineon.com/AN215656) – PSOC&trade; 6 MCU: Dual-CPU system design
Code examples  | [Using ModusToolbox&trade;](https://github.com/Infineon/Code-Examples-for-ModusToolbox-Software) on GitHub
Device documentation | [PSOC&trade; 6 MCU datasheets](https://documentation.infineon.com/html/psoc6/bnm1651211483724.html) <br> [PSOC&trade; 6 technical reference manuals](https://documentation.infineon.com/html/psoc6/zrs1651212645947.html)
Development kits | Select your kits from the [Evaluation board finder](https://www.infineon.com/cms/en/design-support/finder-selection-tools/product-finder/evaluation-board)
Libraries on GitHub  | [mtb-pdl-cat1](https://github.com/Infineon/mtb-pdl-cat1) – PSOC&trade; 6 Peripheral Driver Library (PDL)  <br> [mtb-hal-cat1](https://github.com/Infineon/mtb-hal-cat1) – Hardware Abstraction Layer (HAL) library <br> [retarget-io](https://github.com/Infineon/retarget-io) – Utility library to retarget STDIO messages to a UART port
Middleware on GitHub  | [MCUboot](https://github.com/mcu-tools/mcuboot) – Open-source library enabling the development of secure bootloader applications for 32-bit MCUs <br> [ota-update](https://github.com/Infineon/ota-update) – OTA library and docs <br> [wifi-mw-core](https://github.com/Infineon/wifi-mw-core) – Wi-Fi middleware core library and docs <br> [capsense](https://github.com/Infineon/capsense) – CAPSENSE&trade; library and documents <br> [psoc6-middleware](https://github.com/Infineon/modustoolbox-software#psoc-6-middleware-libraries) – Links to all PSOC&trade; 6 MCU middleware
Tools  | [ModusToolbox&trade;](https://www.infineon.com/modustoolbox) – ModusToolbox&trade; software is a collection of easy-to-use libraries and tools enabling rapid development with Infineon MCUs for applications ranging from wireless and cloud-connected systems, edge AI/ML, embedded sense and control, to wired USB connectivity using PSOC&trade; Industrial/IoT MCUs, AIROC&trade; Wi-Fi and Bluetooth&reg; connectivity devices, XMC&trade; Industrial MCUs, and EZ-USB&trade;/EZ-PD&trade; wired connectivity controllers. ModusToolbox&trade; incorporates a comprehensive set of BSPs, HAL, libraries, configuration tools, and provides support for industry-standard IDEs to fast-track your embedded application development

<br>


## Other resources

Infineon provides a wealth of data at [www.infineon.com](https://www.infineon.com) to help you select the right device, and quickly and effectively integrate it into your design.


## Document history

Document title: *CE230815* – *PSOC&trade; 6 MCU: MCUboot-based bootloader with rollback to factory app*

 Version | Description of change
 ------- | ---------------------
 1.0.0   | New code example
 2.0.0   | Updated to: <br>1. Support anycloud-ota v4.X library <br>2. Used locally installed Mosquitto broker
 2.1.0   | Updated to support ModusToolbox&trade; v2.4 <br>Fixed minor bugs
 3.0.0   | Updated the example to use mcuboot v1.8.1 and ota-update v2.0.0 library <br> Updated to support ModusToolbox&trade; v3.0
 3.1.0   | Fixed minor bugs
 4.0.0   | Added support for the CY8CEVAL-062S2-LAI-4373M2 KIT
 4.0.1   | Minor Makefile changes
 4.1.0   | Updated to support ModusToolbox&trade; v3.2
 5.0.0   | Updated to support MCUboot middleware v1.9.1 <br> Updated to support ota-update middleware v4.X <br> Updated to support ModusToolbox&trade; v3.4 <br> Added support for CY8CPROTO-062S2-43439, CY8CEVAL-062S2-LAI-43439M2, CY8CEVAL-062S2-MUR-43439M2, CY8CEVAL-062S2-MUR-4373EM2, CY8CEVAL-062S2-MUR-4373M2
<br>


All referenced product or service names and trademarks are the property of their respective owners.

The Bluetooth&reg; word mark and logos are registered trademarks owned by Bluetooth SIG, Inc., and any use of such marks by Infineon is under license.

PSOC&trade;, formerly known as PSoC&trade;, is a trademark of Infineon Technologies. Any references to PSoC&trade; in this document or others shall be deemed to refer to PSOC&trade;.

---------------------------------------------------------

© Cypress Semiconductor Corporation, 2021-2025. This document is the property of Cypress Semiconductor Corporation, an Infineon Technologies company, and its affiliates ("Cypress").  This document, including any software or firmware included or referenced in this document ("Software"), is owned by Cypress under the intellectual property laws and treaties of the United States and other countries worldwide.  Cypress reserves all rights under such laws and treaties and does not, except as specifically stated in this paragraph, grant any license under its patents, copyrights, trademarks, or other intellectual property rights.  If the Software is not accompanied by a license agreement and you do not otherwise have a written agreement with Cypress governing the use of the Software, then Cypress hereby grants you a personal, non-exclusive, nontransferable license (without the right to sublicense) (1) under its copyright rights in the Software (a) for Software provided in source code form, to modify and reproduce the Software solely for use with Cypress hardware products, only internally within your organization, and (b) to distribute the Software in binary code form externally to end users (either directly or indirectly through resellers and distributors), solely for use on Cypress hardware product units, and (2) under those claims of Cypress's patents that are infringed by the Software (as provided by Cypress, unmodified) to make, use, distribute, and import the Software solely for use with Cypress hardware products.  Any other use, reproduction, modification, translation, or compilation of the Software is prohibited.
<br>
TO THE EXTENT PERMITTED BY APPLICABLE LAW, CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARD TO THIS DOCUMENT OR ANY SOFTWARE OR ACCOMPANYING HARDWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  No computing device can be absolutely secure.  Therefore, despite security measures implemented in Cypress hardware or software products, Cypress shall have no liability arising out of any security breach, such as unauthorized access to or use of a Cypress product. CYPRESS DOES NOT REPRESENT, WARRANT, OR GUARANTEE THAT CYPRESS PRODUCTS, OR SYSTEMS CREATED USING CYPRESS PRODUCTS, WILL BE FREE FROM CORRUPTION, ATTACK, VIRUSES, INTERFERENCE, HACKING, DATA LOSS OR THEFT, OR OTHER SECURITY INTRUSION (collectively, "Security Breach").  Cypress disclaims any liability relating to any Security Breach, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any Security Breach.  In addition, the products described in these materials may contain design defects or errors known as errata which may cause the product to deviate from published specifications. To the extent permitted by applicable law, Cypress reserves the right to make changes to this document without further notice. Cypress does not assume any liability arising out of the application or use of any product or circuit described in this document. Any information provided in this document, including any sample design information or programming code, is provided only for reference purposes.  It is the responsibility of the user of this document to properly design, program, and test the functionality and safety of any application made of this information and any resulting product.  "High-Risk Device" means any device or system whose failure could cause personal injury, death, or property damage.  Examples of High-Risk Devices are weapons, nuclear installations, surgical implants, and other medical devices.  "Critical Component" means any component of a High-Risk Device whose failure to perform can be reasonably expected to cause, directly or indirectly, the failure of the High-Risk Device, or to affect its safety or effectiveness.  Cypress is not liable, in whole or in part, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any use of a Cypress product as a Critical Component in a High-Risk Device. You shall indemnify and hold Cypress, including its affiliates, and its directors, officers, employees, agents, distributors, and assigns harmless from and against all claims, costs, damages, and expenses, arising out of any claim, including claims for product liability, personal injury or death, or property damage arising from any use of a Cypress product as a Critical Component in a High-Risk Device. Cypress products are not intended or authorized for use as a Critical Component in any High-Risk Device except to the limited extent that (i) Cypress's published data sheet for the product explicitly states Cypress has qualified the product for use in a specific High-Risk Device, or (ii) Cypress has given you advance written authorization to use the product as a Critical Component in the specific High-Risk Device and you have signed a separate indemnification agreement.
<br>
Cypress, the Cypress logo, and combinations thereof, ModusToolbox, PSoC, CAPSENSE, EZ-USB, F-RAM, and TRAVEO are trademarks or registered trademarks of Cypress or a subsidiary of Cypress in the United States or in other countries. For a more complete list of Cypress trademarks, visit www.infineon.com. Other names and brands may be claimed as property of their respective owners.
