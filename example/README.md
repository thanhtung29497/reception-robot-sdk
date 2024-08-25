# smart_robot_example

Demonstrates how to use the smart_robot plugin.

- The example app is located in the `example` folder. All example-related code is in the `example/lib/main.dart` file.
- To run the example app, open the terminal and run the following command:
```bash
cd example
flutter run
```
- The example app will start and you can see 4 buttons: `Start Trigger Word`, `Stop Trigger Word`, `Start VAD`, `Stop VAD`. You can use these buttons to control the plugin.
- When pressing the `Start Trigger Word` button, the plugin will start detecting the trigger word.
    - The trigger word is `Vừng ơi`
    - When triggering the word, the example app will display a dialog `Trigger word detected`
    - The plugin will start the VAD model to capture the audio data. The audio data will be sent to the server.
    - When receiving the result message from the server, the example app will print the message to the console.
    - The plugin will stop the VAD model when the silence timeout is reached and display a dialog `Silence timeout`. Additionally, the plugin will start detecting the trigger word again.
- When pressing the `Stop Trigger Word` button, the plugin will stop detecting the trigger word.
- When pressing the `Start VAD` button, the app will do exactly the same as when the trigger word is detected.
- When pressing the `Stop VAD` button, the plugin will stop the VAD model.