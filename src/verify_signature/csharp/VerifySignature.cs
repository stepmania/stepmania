// Based on http://www.jensign.com/VerifySignature/dotnet/JKeyNet/
// Parameters:
//    public_key - X.509 standard SubjectPublicKeyInfo key in binary format
//    data_file - the data file whose signature to verify
//    signature - the signature of data_file in binary format
using System;
using System.IO;
using System.Text;
using System.Security;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Runtime.InteropServices;

namespace VerifySignature 
{

	//--- P/Invoke CryptoAPI wrapper classes -----
	public class Win32 
	{

		[DllImport("crypt32.dll")]
		public static extern bool CryptDecodeObject(
			uint CertEncodingType,
			uint lpszStructType,
			byte[] pbEncoded,
			uint cbEncoded,
			uint flags,
			[In, Out] byte[] pvStructInfo,
			ref uint cbStructInfo);


		[DllImport("crypt32.dll")]
		public static extern bool CryptDecodeObject(
			uint CertEncodingType,
			uint lpszStructType,
			byte[] pbEncoded,
			uint cbEncoded,
			uint flags,
			IntPtr pvStructInfo,
			ref uint cbStructInfo);
	}


	[StructLayout(LayoutKind.Sequential)]
	public struct PUBKEYBLOBHEADERS 
	{
		public byte bType;	//BLOBHEADER
		public byte bVersion;	//BLOBHEADER
		public short reserved;	//BLOBHEADER
		public uint aiKeyAlg;	//BLOBHEADER
		public uint magic;	//RSAPUBKEY
		public uint bitlen;	//RSAPUBKEY
		public uint pubexp;	//RSAPUBKEY
	}


	[StructLayout(LayoutKind.Sequential)]
	public struct CERT_PUBLIC_KEY_INFO
	{
		public IntPtr SubjPKIAlgpszObjId;
		public int SubjPKIAlgParameterscbData;
		public IntPtr SubjPKIAlgParameterspbData;
		public int PublicKeycbData;
		public IntPtr PublicKeypbData;
		public int PublicKeycUnusedBits;
	}


	public class RSAPubKeyData
	{

		const uint X509_ASN_ENCODING 		= 0x00000001;
		const uint PKCS_7_ASN_ENCODING 	= 0x00010000;

		const uint RSA_CSP_PUBLICKEYBLOB	= 19;
		const uint X509_PUBLIC_KEY_INFO	= 8;

		const int  AT_KEYEXCHANGE		= 1;  //keyspec values
		const int  AT_SIGNATURE		= 2;
		static uint ENCODING_TYPE 		= PKCS_7_ASN_ENCODING | X509_ASN_ENCODING ;

		const byte PUBLICKEYBLOB	= 	0x06;
		const byte CUR_BLOB_VERSION	= 	0x02;
		const ushort reserved 		= 	0x0000;
		const uint CALG_RSA_KEYX 	= 	0x0000a400;
		const uint CALG_RSA_SIGN 	= 	0x00002400;


		private byte[] keyModulus;	// big-Endian 
		private byte[] keyExponent;	// big-Endian
		private byte[] publicKeyBlob;	//Microsoft PUBLICKEYBLOB format
		private uint keySize;		//modulus size in bits
		private bool verbose = false;


		public uint keysize
		{
			get{return keySize;}
		}

		public byte[] keymodulus
		{
			get{return keyModulus;}
		}

		public byte[] keyexponent
		{
			get{return keyExponent;}
		}

		public byte[] MSpublickeyblob
		{
			get{return publicKeyBlob;}
		}

		public static string readFile(string file)
		{
			string finalStr="";
			try 
			{
				// Create an instance of StreamReader to read from a file.
				// The using statement also closes the StreamReader.
				using (StreamReader sr = new StreamReader(file)) 
				{
					String line;
					// Read and display lines from the file until the end of 
					// the file is reached.
					while ((line = sr.ReadLine()) != null) 
					{
						//Console.WriteLine(line);	
						finalStr = finalStr+line;
					}
					return finalStr;
				}
			}
			catch (Exception e) 
			{
				// Let the user know what went wrong.
				Console.WriteLine("The file could not be read:");
				Console.WriteLine(e.Message);
			}
			return null;
		}

		public static void Main(String[] args)
		{
			RSAPubKeyData orsakey = new RSAPubKeyData();

			if(args.Length<3)
			{
				Console.WriteLine("\nUsage:    RSAPubKeyData.exe public_key data_fn signature_fn");
				return;
			}

			String publickeyfn = args[0];
			String datafn = args[1];
			String signaturefn = args[2];

			if (!File.Exists(publickeyfn))
			{
				Console.WriteLine("File '{0}' not found.", publickeyfn);
				return;
			}
			if (!File.Exists(datafn))
			{
				Console.WriteLine("File '{0}' not found.", datafn);
				return;
			}
			if (!File.Exists(signaturefn))
			{
				Console.WriteLine("File '{0}' not found.", signaturefn);
				return;
			}

			Console.WriteLine("\n\n-------- Trying to decode keyfile as  X.509 SubjectPublicKeyInfo format --------");
			if(!orsakey.DecodeSubjectPublicKeyInfo(publickeyfn))
			{
				Console.WriteLine("FAILED to decode as X.509 SubjectPublicKeyInfo");
				return;
			}

			Console.WriteLine("Decoded successfully as X.509 SubjectPublicKeyInfo");

			RSAParameters RSAKeyInfo = new RSAParameters();
			RSACryptoServiceProvider RSA = new RSACryptoServiceProvider();
			
			RSAKeyInfo.Modulus = orsakey.keymodulus;
			RSAKeyInfo.Exponent = orsakey.keyexponent;
			RSA.ImportParameters(RSAKeyInfo);
			
			byte[] data = GetFileBytes(datafn);
			byte[] signature = GetFileBytes(signaturefn);

			if(RSA.VerifyData(data,"SHA1",signature))
			{
				Console.WriteLine("The signature is valid.");
			}
			else
			{
				Console.WriteLine("The signature is not valid.");
			}
		}


		//----  RSAPublicKey,  PKCS #1 format  -----
		public bool DecodeRSAPublicKey(String RSAPublicKeyfile)
		{
			if (!File.Exists(RSAPublicKeyfile))
				return false;
			byte[] encodeddata = GetFileBytes(RSAPublicKeyfile);
			return DecodeRSAPublicKey(encodeddata);
		}




		//----  SubjectPublicKeyInfo,  X.509 standard format; e.g. Java getEncoded(); OpenSSL exported etc.
		// ---  decode first to RSAPublicKey encoded format ----
		public bool DecodeSubjectPublicKeyInfo(String SubjectPublicKeyInfoFile)
		{
			if (!File.Exists(SubjectPublicKeyInfoFile))
				return false;
			byte[] subjectpublickeydata = GetFileBytes(SubjectPublicKeyInfoFile);

			IntPtr pcertpublickeyinfo = IntPtr.Zero ;
			uint cbytes=0;
			if(Win32.CryptDecodeObject(ENCODING_TYPE, X509_PUBLIC_KEY_INFO, subjectpublickeydata, (uint)subjectpublickeydata.Length, 0, IntPtr.Zero, ref cbytes))
			{
				pcertpublickeyinfo = Marshal.AllocHGlobal((int)cbytes);
				Win32.CryptDecodeObject(ENCODING_TYPE, X509_PUBLIC_KEY_INFO, subjectpublickeydata, (uint)subjectpublickeydata.Length, 0, pcertpublickeyinfo, ref cbytes);
				CERT_PUBLIC_KEY_INFO pkinfo = (CERT_PUBLIC_KEY_INFO) Marshal.PtrToStructure(pcertpublickeyinfo, typeof(CERT_PUBLIC_KEY_INFO) );
				IntPtr pencodeddata	= pkinfo.PublicKeypbData;
				int cblob		= pkinfo.PublicKeycbData ;
				byte[] encodeddata	= new byte[cblob];
				Marshal.Copy(pencodeddata, encodeddata, 0,cblob) ;  //copy bytes from IntPtr to byte[]
				Marshal.FreeHGlobal(pcertpublickeyinfo) ;
				return  DecodeRSAPublicKey(encodeddata);
			}
			else
			{
				return false;
			}
		}





		//----- decode public key and extract modulus and exponent from RSAPublicKey,  PKCS #1 format byte[] ----
		public bool DecodeRSAPublicKey(byte[] encodedpubkey)
		{
			byte[] publickeyblob ;

			uint blobbytes=0;
			if(Win32.CryptDecodeObject(ENCODING_TYPE, RSA_CSP_PUBLICKEYBLOB, encodedpubkey, (uint)encodedpubkey.Length, 0, null, ref blobbytes))
			{
				publickeyblob = new byte[blobbytes];
				if(Win32.CryptDecodeObject(ENCODING_TYPE, RSA_CSP_PUBLICKEYBLOB, encodedpubkey, (uint)encodedpubkey.Length, 0, publickeyblob, ref blobbytes))
					if(verbose)
						showBytes("CryptoAPI publickeyblob", publickeyblob);
			}
			else
			{
				return false;
			}
			this.publicKeyBlob = publickeyblob;
			return DecodeMSPublicKeyBlob(publickeyblob);
		}



		//----  Microsoft PUBLICKEYBLOB format  -----
		public bool DecodeMSPublicKeyBlob(String publickeyblobfile)
		{
			if (!File.Exists(publickeyblobfile))
				return false;
			byte[] publickeyblobdata = GetFileBytes(publickeyblobfile);
			return DecodeMSPublicKeyBlob(publickeyblobdata);
		}



		//-----  Microsoft PUBLICKEYBLOB format  ----
		public bool DecodeMSPublicKeyBlob(byte[] publickeyblob)
		{
			PUBKEYBLOBHEADERS pkheaders = new PUBKEYBLOBHEADERS() ;
			int headerslength = Marshal.SizeOf(pkheaders);
			IntPtr buffer = Marshal.AllocHGlobal( headerslength);
			Marshal.Copy( publickeyblob, 0, buffer, headerslength );
			pkheaders = (PUBKEYBLOBHEADERS) Marshal.PtrToStructure( buffer, typeof(PUBKEYBLOBHEADERS) );
			Marshal.FreeHGlobal( buffer );

			//-----  basic sanity check of PUBLICKEYBLOB fields ------------
			if(pkheaders.bType 	!= PUBLICKEYBLOB)
				return false;
			if(pkheaders.bVersion 	!= CUR_BLOB_VERSION)
				return false;
			if(pkheaders.aiKeyAlg 	!= CALG_RSA_KEYX &&  pkheaders.aiKeyAlg != CALG_RSA_SIGN)
				return false;

			if(verbose) 
			{
				Console.WriteLine("\n ---- PUBLICKEYBLOB headers ------");
				Console.WriteLine("  btype     {0}", pkheaders.bType);
				Console.WriteLine("  bversion  {0}", pkheaders.bVersion);
				Console.WriteLine("  reserved  {0}", pkheaders.reserved);
				Console.WriteLine("  aiKeyAlg  0x{0:x8}", pkheaders.aiKeyAlg);
				String magicstring = (new ASCIIEncoding()).GetString(BitConverter.GetBytes(pkheaders.magic)) ;
				Console.WriteLine("  magic     0x{0:x8}     '{1}'", pkheaders.magic, magicstring);
				Console.WriteLine("  bitlen    {0}", pkheaders.bitlen);
				Console.WriteLine("  pubexp    {0}", pkheaders.pubexp);
				Console.WriteLine(" --------------------------------");
			}
			//-----  Get public key size in bits -------------
			this.keySize = pkheaders.bitlen;

			//-----  Get public exponent -------------
			byte[] exponent = BitConverter.GetBytes(pkheaders.pubexp); //little-endian ordered
			Array.Reverse(exponent);    //convert to big-endian order
			this.keyExponent = exponent;
			if(verbose)
				showBytes("\nPublic key exponent (big-endian order):", exponent);

			//-----  Get modulus  -------------
			int modulusbytes = (int)pkheaders.bitlen/8 ;
			byte[] modulus = new byte[modulusbytes];
			try
			{
				Array.Copy(publickeyblob, headerslength, modulus, 0, modulusbytes);
				Array.Reverse(modulus);   //convert from little to big-endian ordering.
				this.keyModulus = modulus;
				if(verbose)
					showBytes("\nPublic key modulus  (big-endian order):", modulus);
			}
			catch(Exception)
			{
				Console.WriteLine("Problem getting modulus from publickeyblob");
				return false;
			}
			return true;
		}


		private static byte[] GetFileBytes(String filename)
		{
			if(!File.Exists(filename))
				return null;
			Stream stream=new FileStream(filename,FileMode.Open);
			int datalen = (int)stream.Length;
			byte[] filebytes =new byte[datalen];
			stream.Seek(0,SeekOrigin.Begin);
			stream.Read(filebytes,0,datalen);
			stream.Close();
			return filebytes;
		}


		private void PutFileBytes(String outfile, byte[] data, int bytes) 
		{
			FileStream fs = null;
			if(bytes > data.Length) 
			{
				Console.WriteLine("Too many bytes");
				return;
			}
			try
			{
				fs = new FileStream(outfile, FileMode.Create);
				fs.Write(data, 0, bytes);
			}
			catch(Exception e) 
			{
				Console.WriteLine(e.Message) ; 
			}
			finally 
			{
				fs.Close();
			}
		}


		private static void showBytes(String info, byte[] data)
		{
			Console.WriteLine("{0}  [{1} bytes]", info, data.Length);
			for(int i=1; i<=data.Length; i++)
			{	
				Console.Write("{0:X2}  ", data[i-1]) ;
				if(i%16 == 0)
					Console.WriteLine();
			}
			Console.WriteLine();
		}

	}
}
