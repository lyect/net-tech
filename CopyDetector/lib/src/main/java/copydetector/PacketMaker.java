package copydetector;

import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

public class PacketMaker {
	
	public static String multicastAddressString;
	public static int key;
	
	public static final DatagramPacket makePacket(int nonce) throws UnknownHostException {
		
		ByteBuffer byteContent = ByteBuffer.allocate(8);

		byteContent.putInt(GreatHasher.hashFunction2(nonce, key));
		byteContent.putInt(nonce);

		DatagramPacket datagram = new DatagramPacket(
			byteContent.array(),
			byteContent.array().length,
			InetAddress.getByName(multicastAddressString),
			8000
		);

		return datagram;
	}
}
