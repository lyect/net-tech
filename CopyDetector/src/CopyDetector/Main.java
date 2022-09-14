package CopyDetector;

import java.util.Scanner;

public class Main {

	public static void main(String[] args) throws Exception {

		if (args.length == 0) {
			throw new Exception("Not enough arguments");
		}

		if (args.length > 1) {
			throw new Exception("Too much arguments");
		}

		System.out.println("I am here!");

		CopyDetector copyDetector = new CopyDetector();

		copyDetector.connect(args[0]); // send packet to multicast address
		copyDetector.hello(); // send packet via multicast
		copyDetector.listen(); // start new thread with handler

		System.out.println("Enter \"STOP\", when you want to stop");
		Scanner scanner = new Scanner(System.in);
		while (true) {
			String message = scanner.nextLine();
			System.out.println("Got from keyboard: " + message);
			if (message.equals("STOP")) {
				break;
			}
		}

		scanner.close();
		copyDetector.goodbye();
	}
}
