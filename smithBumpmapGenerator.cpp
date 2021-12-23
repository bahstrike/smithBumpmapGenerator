#include "smith.h"
#include <minmax.h>
#include <math.h>

extern "C"
{
	SMITHCALLS* smith = nullptr;

	int __cdecl SmithQueryPlugin(PLUGININFO& p)
	{
		strcpy(p.name, "Bumpmap Generator");
		strcpy(p.author, "bahstrike");
		strcpy(p.authorEmail, "strike@bah.wtf");
		strcpy(p.attributions, "DeepMind&www.deepmind.com|Id Software, Inc.&www.idsoftware.com|Google, Inc.&www.google.com");
		strcpy(p.desc, "Generates normal maps from RGB textures.");
		strcpy(p.homepageURL, "https://github.com/bahstrike/smithBumpmapGenerator");
		p.smithRequiredVer = SMITHVERSION;
		p.ver = 100;
		p.purpose = PP_GENERATENORMAL;

		//p.authoritykey

		return 1337;
	}

	int __cdecl InitializePlugin(SMITHCALLS* _smith)
	{
		smith = _smith;

		


		return true;
	}

	void __cdecl ShutdownPlugin()
	{
		

		smith = nullptr;
	}


	void RGBAtoNormal(const byte *in, byte *out, int width, int height, bool clampToEdge);
	void __cdecl OnGenerateNormalMap(unsigned char* input, unsigned char* output, int width, int height, bool clampToEdge)
	{
		RGBAtoNormal(input, output, width, height, clampToEdge);
	}



	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	// start of code from:   https://github.com/deepmind/lab/blob/cf2f5250e1a00ecce37b3480df28c3a5dcd08b57/engine/code/renderergl2/tr_image.c#L428
	// modifications are notated with  Strike:
	//-----------------------------------------------------------------------------------------------------------------------------------------------------


	// Strike: just define a CLAMP macro here  (didnt bother to track down original)
#define CLAMP(i, mn, mx)  ( (i)<(mn) ? (mn) : ((i)>(mx) ? (mx) : (i)) )

	// Strike: just define a normalize func here  (didnt bother to track down original)
	bool VectorNormalize2(double* fIn, double* fOut)
	{
		float d = sqrt( fIn[0]*fIn[0] + fIn[1]*fIn[1] + fIn[2]*fIn[2] );

		if (d == 0.0)
			return false;

		fOut[0] = fIn[0] / d;
		fOut[1] = fIn[1] / d;
		fOut[2] = fIn[2] / d;

		return true;
	}

	byte FloatToOffsetByte(double a)
	{
		return (byte)((a + 1.0) * 127.5);
	}

	// uses a sobel filter to change a texture to a normal map
	void RGBAtoNormal(const byte *in, byte *out, int width, int height, bool clampToEdge)
	{
		int x, y, max;

		// convert to heightmap, storing in alpha
		// same as converting to Y in YCoCg
		max = 1;
		for (y = 0; y < height; y++)
		{
			const byte *inbyte = in + y * width * 4;
			byte       *outbyte = out + y * width * 4 + 3;

			for (x = 0; x < width; x++)
			{
				byte result = (inbyte[0] >> 2) + (inbyte[1] >> 1) + (inbyte[2] >> 2);
				result = result * result / 255; // Make linear
				*outbyte = result;
				max = __max(max, *outbyte);//Strike: different MAX macro
				outbyte += 4;
				inbyte += 4;
			}
		}

		// level out heights
		if (max < 255)
		{
			for (y = 0; y < height; y++)
			{
				byte *outbyte = out + y * width * 4 + 3;

				for (x = 0; x < width; x++)
				{
					*outbyte = *outbyte + (255 - max);
					outbyte += 4;
				}
			}
		}


		// now run sobel filter over height values to generate X and Y
		// then normalize
		for (y = 0; y < height; y++)
		{
			byte *outbyte = out + y * width * 4;

			for (x = 0; x < width; x++)
			{
				// 0 1 2
				// 3 4 5
				// 6 7 8

				byte s[9];
				int x2, y2, i;
				double normal[3];//Strike: just use a double array instead of  vec3_t  which we didnt include here

				i = 0;
				for (y2 = -1; y2 <= 1; y2++)
				{
					int src_y = y + y2;

					if (clampToEdge)
					{
						src_y = CLAMP(src_y, 0, height - 1);
					}
					else
					{
						src_y = (src_y + height) % height;
					}


					for (x2 = -1; x2 <= 1; x2++)
					{
						int src_x = x + x2;

						if (clampToEdge)
						{
							src_x = CLAMP(src_x, 0, width - 1);
						}
						else
						{
							src_x = (src_x + width) % width;
						}

						s[i++] = *(out + (src_y * width + src_x) * 4 + 3);
					}
				}

				normal[0] = s[0] - s[2]
					+ 2 * s[3] - 2 * s[5]
					+ s[6] - s[8];

				normal[1] = s[0] + 2 * s[1] + s[2]

					- s[6] - 2 * s[7] - s[8];

				normal[2] = s[4] * 4;


				// Strike:  add some boost to x/y
				const double normalMapBoost = 4.0;
				normal[0] *= normalMapBoost;
				normal[1] *= normalMapBoost;


				if (!VectorNormalize2(normal, normal))
				{
					// Strike:  just set values manually instead of using original function
					//VectorSet(normal, 0, 0, 1);
					normal[0] = 0;
					normal[1] = 0;
					normal[2] = 1;
				}

				// Strike:  outputting in reverse;  cant remember why, but it makes it work :)
				*outbyte++ = FloatToOffsetByte(normal[2]);
				*outbyte++ = FloatToOffsetByte(normal[1]);
				*outbyte++ = FloatToOffsetByte(normal[0]);
				outbyte++;
			}
		}
	}


}