from connectDAWsMock import ConnectDAWsMock


if __name__ == "__main__":
    # Example configuration
    ip = "::1"  # IPv6 loopback address
    port = 7000  # Example port used by ConnectDAWs
    connectDAWsMock = ConnectDAWsMock(
        ip=ip,
        provider_port=8001,
        consumer_port=8002,
        host_port=7000,
        sample_rate=44100,
        samples_per_block=512,
        num_input_channels=2,
        num_output_channels=2,
    )

    connectDAWsMock.connect_to_connect_daws(ip, port)
    connectDAWsMock.stream_audio(440)
