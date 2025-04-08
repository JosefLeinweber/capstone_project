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
    """Create a configuration dictionary to exchange with ConnectDAWs."""
    return {
        "ip": ip,
        "provider_port": provider_port,
        "consumer_port": consumer_port,
        "host_port": host_port,
        "sample_rate": sample_rate,
        "samples_per_block": samples_per_block,
        "num_input_channels": num_input_channels,
        "num_output_channels": num_output_channels,
    }


def connect_to_connect_daws(ip, port, configuration):
    """Connect to ConnectDAWs and exchange configuration."""
    try:
        # Create a socket
        with socket.socket(socket.AF_INET6, socket.SOCK_STREAM) as client_socket:
            print(f"Connecting to ConnectDAWs at {ip}:{port}...")
            client_socket.connect((ip, port))
            print("Connected!")

            # Send configuration as JSON
            config_json = json.dumps(configuration)
            client_socket.sendall(config_json.encode("utf-8"))
            print("Configuration sent.")

            # Wait for acknowledgment or response
            response = client_socket.recv(1024).decode("utf-8")
            print(f"Response from ConnectDAWs: {response}")

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
