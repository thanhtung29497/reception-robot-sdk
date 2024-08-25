## smart_robot plugin

A Flutter plugin for the smart robot project. This plugin is used to detect the trigger word and capture the speech data.

### Integration Guide

1. Add the plugin to your project folder

    Clone the smart_robot plugin from this repository and add it to your project folder as git submodule.
    ```bash
    git submodule add https://github.com/thanhtung29497/reception-robot-sdk.git
    git submodule update --init --recursive
    ```

2. Import the plugin
    ```dart
    import 'package:smart_robot/smart_robot.dart';
    ```

   If you want to use the AudioEvent
    ```dart
    import 'package:smart_robot/audio_event.dart';
    import 'package:smart_robot/audio_event_listener.dart';
    ```

3. Initialize the plugin
    - Get the platform version
    - Initialize the trigger word model
    - Initialize the VAD model

    ```dart
    final _smartRobotPlugin = SmartRobot();
    
    function initPlugin() async {
      try {
        final platformVersion = await _smartRobotPlugin.getPlatformVersion();
        print('Platform version: $platformVersion');
        await _smartRobotPlugin.initTriggerWordModel();
        await _smartRobotPlugin.initVADModel();
      } on PlatformException {
        print('Failed to initialize the plugin');
      }
    }
    ```

4. Use the plugin
    - Start the trigger word detection
        ```dart
        await _smartRobotPlugin.startTriggerWord()
        ```

5. Listen to the AudioEvent
    - Implement the AudioEventListener interface
    - Add the AudioEventListener to the plugin
    - Override the methods of the AudioEventListener interface
    - Remember to remove the AudioEventListener when the widget is disposed

    ```dart
    class YourClass implements AudioEventListener {
      void initState() {
        super.initState();
        _smartRobotPlugin.addAudioEventListener(this);
      }
    
      @override
      void onTriggerWordDetected() {
        print('Trigger word detected');
        _smartRobotPlugin.startVAD();
      }
      
      @override
      void onSpeaking(VADEvent event) {
        // Send the audio data to the server
      }
      
      @override
      void onSpeechEnd() {
        print('Speech end');
        // Send the ending signal to the server
      }
      
      @override
      void onSilenceTimeout() {
        print('Silence timeout');
        _smartRobotPlugin.stopVAD();
        _smartRobotPlugin.startTriggerWord();
      }
      
      @override
      void dispose() {
        super.dispose();
        _smartRobotPlugin.removeAudioEventListener(this);
        channel?.sink.close();
      }
    }
    ```

### Example App
- The example app is located in the `example` folder. All example-related code is in the `example/lib/main.dart` file.
- See [example/README.md](example/README.md) for more information on how to run the example app.




