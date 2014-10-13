package com.net.rpc;

import java.io.UnsupportedEncodingException;

public class ByteConvertUtil {
	// basic data type size in bytes
		public final static int BOOLEAN_BYTES = 1;
		public final static int BYTE_BYTES = 1;
		public final static int CHAR_BYTES = 2;	//save in Unicode 
		public final static int SHORT_BYTES = 2;
		public final static int INT_BYTES = 4;
		public final static int LONG_BYTES = 8;
		public final static int FLOAT_BYTES = 4;	//Single precision floating point number
		public final static int DOUBLE_BYTES = 8;	//Double precision floating point number

		
		public static boolean byte2boolean( byte b ) {
			if( 0 != b ){
				return true;
			}
			else{
				return false;
			}
		}
		
		public static byte boolean2byte( boolean b ) {
			if( b ){
				return 1;
			}
			else{
				return 0;
			}
		}

		public static short byteArray2short(byte b[], int off ) {
			int i, j;
			int k = 0;

			j = b.length;
			if (b != null && j >= off) {
				if( j > off + SHORT_BYTES ){
					j = off + SHORT_BYTES;
				}
				for( i = j - 1; i >= off; i-- ){
					k = ( k << 8 ) | ( b[ i ] & 0xff );
				}
			}

			return (short) k;
		}

		public static int byteArray2int(byte b[], int off ) {
			int i, j;
			int k = 0;

			j = b.length;
			if (b != null && j >= off) {
				if( j > off + INT_BYTES ){
					j = off + INT_BYTES;
				}
				for( i = j - 1; i >= off; i-- ){
					k = ( k << 8 ) | ( b[ i ] & 0xff );
				}
			}

			return k;
		}

		public static long byteArray2long( byte b[], int off ) {
			int i, j;
			long k = 0;

			j = b.length;
			if (b != null && j >= off) {
				if( j > off + LONG_BYTES ){
					j = off + LONG_BYTES;
				}
				for( i = j - 1; i >= off; i-- ){
					k = ( k << 8 ) | ( b[ i ] & 0xff );
				}
			}

			return k;
		}

		public static void short2byteArray( short k, byte b[], int off ) {
			int j;

			j = b.length;
			if( b == null || j <= off ){
				return;
			}

			j -= off;
			if( j > SHORT_BYTES ){
				j = SHORT_BYTES;
			}

			switch( j ){
				case 2:
					b[ off + 1 ] = (byte) (( k >> 8 ) & 0x0ff);
				case 1:
					b[ off ] = (byte) (k & 0x0ff);
			}
		}

		public static void int2byteArray(int k, byte b[], int off ) {
			int j;

			j = b.length;
			if( b == null || j <= off ){
				return;
			}

			j -= off;
			if( j > INT_BYTES ){
				j = INT_BYTES;
			}

			switch( j ){
				case 4:
					b[ off + 3 ] = (byte) (( k >> 24 ) & 0x0ff);
				case 3:
					b[ off + 2 ] = (byte) (( k >> 16 ) & 0x0ff);
				case 2:
					b[ off + 1 ] = (byte) (( k >> 8 ) & 0x0ff);
				case 1:
					b[ off ] = (byte) (k & 0x0ff);
			}
		}

		public static void long2byteArray( long k, byte b[], int off ) {
			int j;

			j = b.length;
			if( b == null || j <= off ){
				return;
			}

			j -= off;
			if( j > LONG_BYTES ){
				j = LONG_BYTES;
			}

			switch( j ){
				case 8:
					b[ off + 7 ] = (byte) (( k >> 56 ) & 0x0ff);
				case 7:
					b[ off + 6 ] = (byte) (( k >> 48 ) & 0x0ff);
				case 6:
					b[ off + 5 ] = (byte) (( k >> 40 ) & 0x0ff);
				case 5:
					b[ off + 4 ] = (byte) (( k >> 32 ) & 0x0ff);
				case 4:
					b[ off + 3 ] = (byte) (( k >> 24 ) & 0x0ff);
				case 3:
					b[ off + 2 ] = (byte) (( k >> 16 ) & 0x0ff);
				case 2:
					b[ off + 1 ] = (byte) (( k >> 8 ) & 0x0ff);
				case 1:
					b[ off ] = (byte) (k & 0x0ff);
			}
		}

		public static void ArrayMove( byte b[], int srcOff, int dstOff, int Len ){
			int i, j, k;

			if( null == b || 0 == b.length || Len <= 0 ){
				return;
			}

			if( srcOff > dstOff ){
				if( b.length < srcOff + Len ){
					Len = b.length - srcOff;
				}

				k = srcOff + Len;
				for( i = srcOff, j = dstOff; i < k; i++, j++ ){
					b[ j ] = b[ i ];
				}
			}
			else if( srcOff < dstOff ){
				if( b.length < dstOff + Len ){
					Len = b.length - dstOff;
				}

				k = dstOff + Len - 1;
				for( i = srcOff + Len - 1, j = k; i >= srcOff; i--, j-- ){
					b[ j ] = b[ i ];
				}
			}
		}
		
		public static void String2byteArray( String s, byte[] b ){
			byte[] bArray;
			int l = b.length, i, k;

			bArray = s.getBytes();
			if( l > bArray.length ){
				k = bArray.length;
				for( i = k; i < l; i++ ){
					b[ i ] = 0;
				}
			}
			else{
				k = l;
			}
			for( i = 0; i < k; i++ ){
				b[ i ] = bArray[ i ];
			}
		}
		
		public static void String2byteArray( String s, byte[] b, int off){
			byte[] bArray;
			int l = b.length, i, k;

			bArray = s.getBytes();
			if( l > bArray.length ){
				k = bArray.length;
				for( i = k; i < l; i++ ){
					b[ i ] = 0;
				}
			}
			else{
				k = l;
			}
			for( i = 0; i < k; i++ ){
				b[ off+i ] = bArray[ i ];
			}
		}
		
		public static String byteArray2String( byte[] b ){
			String s;
			s = new String( b );
			return s.substring( 0, s.indexOf( 0 ) );
		}
		
		public static String byteArray2String( byte[] b , int offset, int len){
			String s;
			s = new String( b, offset, len);
			return s;
		}
		
		public static String byteArray2String( byte[] b, String charsetName ){
			String s;
			try {
				s = new String( b, charsetName );
			} catch (UnsupportedEncodingException e) {
				e.printStackTrace();
				s = new String( b );
			}
			return s.substring( 0, s.indexOf( 0 ) );
		}
}
