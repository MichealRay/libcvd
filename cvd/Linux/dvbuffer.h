// -*- c++ -*-
#ifndef __DVBUFFER_H
#define __DVBUFFER_H

#include <cvd/videobuffer.h>
#include <cvd/byte.h>
#include <cvd/rgb.h>
#include <cvd/colourspaces.h>
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#include <vector>

namespace CVD {

/// Internal DVBuffer2 helpers
namespace DC
{
	#ifndef DOXYGEN_IGNORE_INTERNAL
	template<class C> struct cam_type
	{
		static const int mode = Error__type_not_valid_for_camera___Use_byte_or_rgb_of_byte;
		static const int fps  = Error__type_not_valid_for_camera___Use_byte_or_rgb_of_byte;
	};
	
	template<> struct cam_type<yuv411>
	{
		static const int mode = MODE_640x480_YUV411;
		static const double fps  = 30;
	};
	
	template<> struct cam_type<byte>
	{
		static const int mode = MODE_640x480_MONO;
		static const double fps  = 30;
	};
	
	template<> struct cam_type<Rgb<byte> >
	{
		static const int mode = MODE_640x480_RGB;
		static const double  fps  = 15;
	};

	struct raw_frame
	{
		unsigned char* data;
		double timestamp;
		int buffer;
	};
	#endif

	/// Internal (non type-safe) class used by DVBuffer2 to do the actual interfacing with the
	/// Firewire (IEE 1394) video hardware. A wrapper for the libdc1394 library.
	/// Use DVBuffer2 if you want 8-bit greyscale or 24-bit colour.
	class RawDCVideo
	{
		public:
		/// Construct a video buffer
		/// @param camera_no The camera number
		/// @param num_dma_buffers The number of DMA buffers to use
		/// @param bright The brightness correction
		/// @param exposure The exposure correction
		/// @param mode The required mode
		/// @param frame_rate The number of frames per second
			RawDCVideo(int camera_no, int num_dma_buffers, int bright, int exposure, int mode, double frame_rate);
			~RawDCVideo();
			
			/// The size of the VideoFrames returned by this buffer
			ImageRef size();
			/// Returns the next frame from the buffer. This function blocks until a frame is ready.
			VideoFrame<byte>* get_frame();
			/// Tell the buffer that you are finished with this frame
			/// \param f The frame that you are finished with.
			void put_frame(VideoFrame<byte>* f);
			/// Is there a frame waiting in the buffer? This function does not block. 
			bool frame_pending();

			/// Set the camera shutter speed
			/// @param s The requested speed
			void set_shutter(unsigned int s);
			/// Get the camera shutter speed
			unsigned int get_shutter();

			/// Set the camera iris
			/// @param i The requested iris
			void set_iris(unsigned int i );
			/// Get the camera iris
			unsigned int get_iris();

			/// Set the camera gain
			/// @param g The requested gain
			void set_gain(unsigned int g);
			/// Get the camera iris
			unsigned int get_gain();

			/// Set the camera exposure
			/// @param e The requested exposure
			void set_exposure(unsigned int e);
			/// Get the camera iris
			unsigned int get_exposure();

			/// Set the camera brightness
			/// @param b The requested brightness
			void set_brightness(unsigned int b);
			/// Get the camera iris
			unsigned int get_brightness();

			/// Get the camera frame rate
			double frame_rate();

			
			/// What is the handle for this device?
			raw1394handle_t& handle();
			/// Which node is this device on?
			nodeid_t&		 node();

		private:

			// components needed for the DMA based video capture
			int my_channel;
			unsigned char* my_ring_buffer;
			int my_frame_size;
			int my_num_buffers;
			// int my_most_recent_frame; // the most recently filled buffer
			int my_fd; // fd of the mmaped dma ring buffer
			raw1394handle_t my_handle;
			nodeid_t * my_camera_nodes; // member variable I guess unless we can make it be released
			nodeid_t my_node;
			ImageRef my_size;

			std::vector<int> my_frame_sequence;
			int my_next_frame;
			int my_last_in_sequence;
			double true_fps;
	};
		
}

/// A video buffer from a Firewire (IEEE 1394) camera.
/// @param T The pixel type of the frames. Currently only <code><CVD::Rgb<CVD::byte> ></code> 
/// and <code>CVD::byte></code> are supported.
/// @ingroup gVideoBuffer
template<class T> class DVBuffer2: public VideoBuffer<T>, public DC::RawDCVideo
{
	public:
		/// Construct a video buffer
		/// @param cam_no The camera number
		/// @param num_dma_buffers The number of DMA buffers to use
		/// @param bright The brightness correction (default = -1 = automatic)
		/// @param exposure The exposure correction (default = -1 = automatic)
		/// @param fps The number of frames per second (default = 30fps)
		DVBuffer2(int cam_no, int num_dma_buffers, int bright=-1, int exposure=-1, double fps=DC::cam_type<T>::fps)
		:RawDCVideo(cam_no, num_dma_buffers, bright, exposure, DC::cam_type<T>::mode, fps)
		{
		}

		virtual ~DVBuffer2()
		{
		}

		double frame_rate()
		{
			return RawDCVideo::frame_rate();
		}

		virtual ImageRef size()
		{
			return RawDCVideo::size();
		}

		virtual VideoFrame<T>* get_frame()
		{
			return reinterpret_cast<VideoFrame<T>*>(RawDCVideo::get_frame());
		}
		
		virtual void put_frame(VideoFrame<T>* f)
		{
			RawDCVideo::put_frame(reinterpret_cast<VideoFrame<byte>*>(f));
		}

		virtual bool frame_pending()
		{
			return RawDCVideo::frame_pending();
		}
	
		virtual void seek_to(double t){}
};

/// An 8-bit greyscale video buffer from a Firewire (IEEE 1394) camera.
/// Provides frames of type DVFrame.
/// @ingroup gVideoBuffer
typedef DVBuffer2<byte> DVBuffer;

}

#endif
