import socket
import time
import numpy as np

# Import the generated Protobuf module
try:
    import datagram_pb2
except ImportError as e:
    print(f"Error importing generated Protobuf module: {e}")
    raise


class ConnectDAWsMock:
    def __init__(
        self,
        ip,
        host_port,
        provider_port,
        consumer_port,
        sample_rate,
        samples_per_block,
        num_input_channels,
        num_output_channels,
    ):
        self.ip = ip
        self.host_port = host_port
        self.provider_port = provider_port
        self.consumer_port = consumer_port
        self.sample_rate = sample_rate
        self.samples_per_block = samples_per_block
        self.num_input_channels = num_input_channels
        self.num_output_channels = num_output_channels
        self.configuration = self._create_configuration()
        self.audio_data = None
        self.plugin_socket = None

    def close(self):
        """Close the server socket."""
        if self.server_socket:
            self.server_socket.close()
            print("Mock server closed.")

    def _create_configuration(self):
        """Create a Configuration Protobuf message."""
        config = datagram_pb2.ConfigurationData()
        config.ip = self.ip
        config.provider_port = self.provider_port
        config.consumer_port = self.consumer_port
        config.host_port = self.host_port
        config.sample_rate = self.sample_rate
        config.samples_per_block = self.samples_per_block
        config.num_input_channels = self.num_input_channels
        config.num_output_channels = self.num_output_channels
        return config

    def connect_to_connect_daws(ip, port, configuration):
        """Connect to ConnectDAWs and exchange configuration."""
        try:
            # Create a socket
            with socket.socket(socket.AF_INET6, socket.SOCK_STREAM) as client_socket:
                print(f"Connecting to ConnectDAWs at {ip}:{port}...")
                client_socket.connect((ip, port))
                print("Connected!")

                # Serialize configuration using Protobuf
                config_bytes = configuration.SerializeToString()
                client_socket.sendall(config_bytes)
                print("Configuration sent.")

                # Wait for acknowledgment or response
                response = client_socket.recv(1024)
                print(f"Response from ConnectDAWs: {response.decode('utf-8')}")

        except Exception as e:
            print(f"An error occurred: {e}")

    def generate_sine_wave(self, frequency, duration, amplitude=0.5):
        """
        Generate a sine wave signal.

        :param frequency: Frequency of the sine wave in Hz
        :param duration: Duration of the sine wave in seconds
        :param amplitude: Amplitude of the sine wave (default: 0.5)
        :return: Numpy array containing the sine wave
        """
        import numpy as np

        t = np.linspace(0, duration, int(self.sample_rate * duration), endpoint=False)
        sine_wave = amplitude * np.sin(2 * np.pi * frequency * t)
        return sine_wave

    def get_plugin_socket(self):
        try:
            self.plugin_socket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
            self.plugin_socket.bind((self.ip, self.host_port))
            return True

        except Exception as e:
            print(f"Error creating client socket: {e}")
            return False

    def stream_audio(self, frequency, duration=5, amplitude=0.5):
        """
        Streams a sine wave audio signal to the plugin for a specified duration.
        :param frequency: Frequency of the sine wave in Hz.
        :param duration: Duration in seconds to stream audio.
        :param amplitude: Amplitude of the sine wave (default: 0.5).
        """
        if not self.get_plugin_socket():
            print("Failed to create plugin socket.")
            return

        try:
            sine_wave = self.generate_sine_wave(frequency, duration, amplitude)
            sine_wave_bytes = sine_wave.tobytes()

            start_time = time.time()
            while time.time() - start_time < duration:
                # Stream the sine wave in chunks
                self.plugin_socket.sendall(sine_wave_bytes[:1024])  # Send 1 KB chunks
                time.sleep(0.01)  # Simulate a small delay between packets
            print("Sine wave audio streaming completed.")
        except Exception as e:
            print(f"Error during sine wave audio streaming: {e}")
