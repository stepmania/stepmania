// Based on http://www.thecodeproject.com/useritems/Porting_Java_Public_Key.asp
// Parameters:
//    public_key - X.509 standard SubjectPublicKeyInfo key in binary format
//    data_file - the data file whose signature to verify
//    signature - the signature of data_file in binary format

/*
 * Title: RSA Security
 * Description: This class generates a RSA private and public key, reinstantiates
 * the keys from the corresponding key files.  It also generates compatible .Net Public Key,
 * which we will read later in C# program using .Net Securtiy Framework
 * The reinstantiated keys are used to sign and verify the given data.</p>
 *
 * @author Shaheryar
 * @version 1.0
 */

import java.security.*;
import java.security.spec.*;
import java.io.*;
import java.security.interfaces.*;
import java.security.cert.*;
import javax.xml.transform.stream.*;
import javax.xml.transform.dom.*;
import javax.xml.transform.*;
import org.w3c.dom.*;
import javax.xml.parsers.*;

public class VerifySignature
{
	private KeyPairGenerator keyGen; // Key pair generator for RSA
	public PrivateKey privateKey; // Private Key Class
	public PublicKey publicKey; // Public Key Class
	public KeyPair keypair; // KeyPair Class
	private Signature sign; // Signature, used to sign the data
	/* Instantiates the key paths and signature algorithm. */
	public VerifySignature()
	{
		try {
			// Get the instance of Signature Engine.
			sign = Signature.getInstance("SHA1withRSA");
		}
		catch (NoSuchAlgorithmException	nsa) {
			System.out.println("" + nsa.getMessage());
		}
	}

	/* Initialize the public and private keys. */
	private void initializePublicKey(String publickey_fn)
	{
		try
		{
			// Read key files back and decode them from BASE64
			byte[] publicKeyBytes	= readKeyBytesFromFile(publickey_fn);

			// Convert back to public and private key objects
			KeyFactory keyFactory	= KeyFactory.getInstance("RSA");
			EncodedKeySpec publicKeySpec = new X509EncodedKeySpec(publicKeyBytes);
			publicKey = keyFactory.generatePublic(publicKeySpec);
		}
		catch (IOException io) {
			System.out.println(
				"Public/Private key file not found."+ io.getCause());
		}
		catch (InvalidKeySpecException e) {
			System.out.println( "Invalid Key Specs. Not valid key files."+ e.getCause() );
		}
		catch (NoSuchAlgorithmException	e) {
			System.out.println( "There is no such algorithm.  Please check the JDK ver."+ e.getCause() );
		}
	}

	/*
	* Verifies the signature for the given bytes using the public key.
	* @param signature Signature
	* @param data Data that was signed
	* @return boolean True if valid signature else false
	*/
	public boolean verifySignature(String publickey_fn, byte[] signature, byte[] data)
	{
		try
		{
			initializePublicKey(publickey_fn);
			sign.initVerify(publicKey);
			sign.update(data);
			return sign.verify(signature);
		}
		catch (SignatureException e) {
			e.printStackTrace();
		}
		catch (InvalidKeyException e) {
		}

		return false;
	}

	/**
	* Returns the contents of the file in a byte array.
	* @param fileName File Name
	* @return byte[] The data read from a given file as a byte array.
	*/
	private byte[] readKeyBytesFromFile(String fileName) throws IOException
	{
		File file = new File(fileName);
		InputStream is = new FileInputStream(file);

		// Get the size of the file
		long length = ile.length();

		// You cannot create an array using a long type.
		// It needs to be an int type.
		// Before converting to an int type, check
		// to ensure that file is not larger than Integer.MAX_VALUE.
		if (length > Integer.MAX_VALUE) {
			// File	is too large
		}

		// Create the byte array to hold the data
		byte[] bytes = new byte[ (int) length];

		// Read in the bytes
		int offset = 0;
		int numRead =	0;
		while	(offset	< bytes.length
			&&	(numRead = is.read(bytes, offset, bytes.length - offset)) >= 0)	{
				offset += numRead;
			}

			// Ensure all the bytes have been read in
			if (offset < bytes.length) {
				throw new IOException("Key File Error: Could not completely read file " + file.getName());
			}

			// Close the input stream and return bytes
			is.close();
			return bytes;
	}


	public static void main(String args[])
	{
		VerifySignature	sm = new VerifySignature();

		/*
		Uncomment next line	for	first time when	you	run	the	code,it	will generate the keys.
		Afterwards,the application will	read the generated key files from the given	location.
		If you want	to generate	the	key	files each time, then you should keep it uncommented always.
		*/
		if( args.length != 3 )
		{
			System.out.println("\nUsage:    RSAPubKeyData.exe publickey_fn data_fn signature_fn");
			return;
		}

		String publickeyfn = args[0];
		String datafn = args[1];
		String signaturefn = args[2];


		byte[] data	= sm.readBytesFromFile(datafn);
		byte[] signature = sm.readBytesFromFile(signaturefn);

		if( sm.verifySignature(publickeyfn,signature,data) )
		{
			System.out.println("The signature is valid.");
		}
		else
		{
			System.out.println("The signature is not valid.");
		}
	}

	private byte[] readBytesFromFile(String fileName)
	{
		try
		{
			File file = new File(fileName);
			InputStream is = new FileInputStream(file);

			// Get the size of the file
			long length = file.length();

			// You cannot create an array	using a	long type.
			// It needs to be an int type.
			// Before converting to an int type, check
			// to ensure that file is not larger than Integer.MAX_VALUE.
			if (length > Integer.MAX_VALUE) {
				// File	is too large
			}

			// Create the byte array to hold the data
			byte[] bytes = new byte[ (int) length];

			// Read in the bytes
			int offset = 0;
			int numRead = 0;
			while (offset < bytes.length
				&& (numRead = is.read(bytes, offset, bytes.length - offset)) >= 0)
			{
				offset += numRead;
			}

			// Ensure all the bytes have been read in
			if (offset < bytes.length) {
				throw new IOException("Key File Error: Could not completely read file " + file.getName());
			}

			// Close the input stream and return bytes
			is.close();
			return bytes;
		}
		catch(IOException ioe)
		{
			System.out.println("Exception occured while writing file"+ioe.getMessage());
		}
		byte[] bytes = new byte[1];
		return bytes; 
	}
}
