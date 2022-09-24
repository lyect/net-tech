package copydetector;

// This hasher has truly great hash functions.
// So, that's the reason why it is called Great.

public class GreatHasher {
	public static final int hashFunction1(int n) {
		return Integer.hashCode(n);
	}
	
	public static final int hashFunction2(int n, int m) {
		int hashPart = hashFunction1(n);
		int hashFull = hashFunction1(hashPart * m);
		return hashFull;
	}
}
