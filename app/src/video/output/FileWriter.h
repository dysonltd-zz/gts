#ifndef AviWriterH
#define AviWriterH

#include <opencv/cv.h>
#include <opencv/highgui.h>

/** @brief Class to write AVI files.
 *
 * A thin wrapper around CvVideoWriter.
 */
class AviWriter
{
public:
    AviWriter( const char* const fileName, const double fps, const int aviWidth, const int aviHeight );
    ~AviWriter();

    void addFrame( const char* const data, const int frameWidth, const int frameHeight );

private:
    IplImage* const CreateImage( const int width, const int height ) const;

    static const int DEFAULT_COMPRESSION_CODEC;
    static const int DEFAULT_NUM_CHANNELS = 3;
    static const int DEFAULT_IMAGE_DEPTH  = IPL_DEPTH_8U;

    int m_aviWidth;       ///< The width of the AVI being generated.
    int m_aviHeight;      ///< The width of the AVI being generated.
    int m_numChannels;    ///< The number of colour channels in the AVI being generated.

    CvVideoWriter* m_avi; ///< OpenCV Video writer.
    IplImage*      m_img; ///< @brief The OpenCV image to use for appending to the AVI
                          ///(to ensure we maintain the frame size/format).
};

#endif // AviWriterH
