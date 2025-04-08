import socket
import json
import google

# Import the generated Protobuf module
try:
    import datagram_pb2
except ImportError as e:
    print(f"Error importing generated Protobuf module: {e}")
    raise


def create_configuration(
    ip,
    provider_port,
    consumer_port,
    host_port,
    sample_rate,
    samples_per_block,
    num_input_channels,
    num_output_channels,
):
    """Create a Configuration Protobuf message."""
    config = datagram_pb2.ConfigurationData()
    config.ip = ip
    config.provider_port = provider_port
    config.consumer_port = consumer_port
    config.host_port = host_port
    config.sample_rate = sample_rate
    config.samples_per_block = samples_per_block
    config.num_input_channels = num_input_channels
    config.num_output_channels = num_output_channels
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


if __name__ == "__main__":
    # Example configuration
    ip = "::1"  # IPv6 loopback address
    port = 7000  # Example port used by ConnectDAWs
    configuration = create_configuration(
        ip=ip,
        provider_port=8001,
        consumer_port=8002,
        host_port=7000,
        sample_rate=44100,
        samples_per_block=512,
        num_input_channels=2,
        num_output_channels=2,
    )

    # Connect and exchange configuration
    connect_to_connect_daws(ip, port, configuration)
