#ifndef EXIFIMAGEHEADER_H
#define EXIFIMAGEHEADER_H

#include "exifvalue.h"
#include <QImage>
#include <QMap>
#include <QList>

class QString;
class QIODevice;
class ExifIfd;
class ExifException;
class ExifMarker;

class ExifImageHeader
{
public:
    enum ImageTag
    {
        ImageWidth                = 0x0100,
        ImageLength               = 0x0101,
        BitsPerSample             = 0x0102,
        Compression               = 0x0103,
        PhotometricInterpretation = 0x0106,
        Orientation               = 0x0112,
        SamplesPerPixel           = 0x0115,
        PlanarConfiguration       = 0x011C,
        YCbCrSubSampling          = 0x0212,
        XResolution               = 0x011A,
        YResolution               = 0x011B,
        ResolutionUnit            = 0x0128,
        StripOffsets              = 0x0111,
        RowsPerStrip              = 0x0116,
        StripByteCounts           = 0x0117,
        TransferFunction          = 0x012D,
        WhitePoint                = 0x013E,
        PrimaryChromaciticies     = 0x013F,
        YCbCrCoefficients         = 0x0211,
        ReferenceBlackWhite       = 0x0214,
        DateTime                  = 0x0132,
        ImageDescription          = 0x010E,
        Make                      = 0x010F,
        Model                     = 0x0110,
        Software                  = 0x0131,
        Artist                    = 0x013B,
        Copyright                 = 0x8298
    };
    enum ExtendedTag
    {
        ExifVersion              = 0x9000,
        FlashPixVersion          = 0xA000,
        ColorSpace               = 0xA001,
        ComponentsConfiguration  = 0x9101,
        CompressedBitsPerPixel   = 0x9102,
        PixelXDimension          = 0xA002,
        PixelYDimension          = 0xA003,
        MakerNote                = 0x927C,
        UserComment              = 0x9286,
        RelatedSoundFile         = 0xA004,
        DateTimeOriginal         = 0x9003,
        DateTimeDigitized        = 0x9004,
        SubSecTime               = 0x9290,
        SubSecTimeOriginal       = 0x9291,
        SubSecTimeDigitized      = 0x9292,
        ImageUniqueId            = 0xA420,

        ExposureTime             = 0x829A,
        FNumber                  = 0x829D,
        ExposureProgram          = 0x8822,
        SpectralSensitivity      = 0x8824,
        ISOSpeedRatings          = 0x8827,
        Oecf                     = 0x8828,
        ShutterSpeedValue        = 0x9201,
        ApertureValue            = 0x9202,
        BrightnessValue          = 0x9203,
        ExposureBiasValue        = 0x9204,
        MaxApertureValue         = 0x9205,
        SubjectDistance          = 0x9206,
        MeteringMode             = 0x9207,
        LightSource              = 0x9208,
        Flash                    = 0x9209,
        FocalLength              = 0x920A,
        SubjectArea              = 0x9214,
        FlashEnergy              = 0xA20B,
        SpatialFrequencyResponse = 0xA20C,
        FocalPlaneXResolution    = 0xA20E,
        FocalPlaneYResolution    = 0xA20F,
        FocalPlaneResolutionUnit = 0xA210,
        SubjectLocation          = 0xA214,
        ExposureIndex            = 0xA215,
		SensingMethod            = 0xA217,
        FileSource               = 0xA300,
        SceneType                = 0xA301,
        CfaPattern               = 0xA302,
        CustomRendered           = 0xA401,
        ExposureMode             = 0xA402,
        WhiteBalance             = 0xA403,
        DigitalZoomRatio         = 0xA404,
        FocalLengthIn35mmFilm    = 0xA405,
        SceneCaptureType         = 0xA406,
        GainControl              = 0xA407,
        Contrast                 = 0xA408,
        Saturation               = 0xA409,
        Sharpness                = 0xA40A,
        DeviceSettingDescription = 0xA40B,
        SubjectDistanceRange     = 0x40C
    };
    enum GpsTag
    {
        GpsVersionId         = 0x0000,
        GpsLatitudeRef       = 0x0001,
        GpsLatitude          = 0x0002,
        GpsLongitudeRef      = 0x0003,
        GpsLongitude         = 0x0004,
        GpsAltitudeRef       = 0x0005,
        GpsAltitude          = 0x0006,
        GpsTimeStamp         = 0x0007,
        GpsSatellites        = 0x0008,
        GpsStatus            = 0x0009,
        GpsMeasureMode       = 0x000A,
        GpsDop               = 0x000B,
        GpsSpeedRef          = 0x000C,
        GpsSpeed             = 0x000D,
        GpsTrackRef          = 0x000E,
        GpsTrack             = 0x000F,
        GpsImageDirectionRef = 0x0010,
        GpsImageDirection    = 0x0011,
        GpsMapDatum          = 0x0012,
        GpsDestLatitudeRef   = 0x0013,
        GpsDestLatitude      = 0x0014,
        GpsDestLongitudeRef  = 0x0015,
        GpsDestLongitude     = 0x0016,
        GpsDestBearingRef    = 0x0017,
        GpsDestBearing       = 0x0018,
        GpsDestDistanceRef   = 0x0019,
        GpsDestDistance      = 0x001A,
        GpsProcessingMethod  = 0x001B,
        GpsAreaInformation   = 0x001C,
        GpsDateStamp         = 0x001D,
        GpsDifferential      = 0x001E
    };
	
	ExifImageHeader();
	explicit ExifImageHeader(const QString &filePath);
    ~ExifImageHeader();
	
	bool loadFromJpeg(const QString &filePath);
    bool loadFromJpeg(QIODevice *device);
    
	bool saveToJpeg(const QString &filePath) const;
    bool saveToJpeg(QIODevice *device) const;
	
	QList<ImageTag> imageTags() const;
    QList<ExtendedTag> extendedTags() const;
    QList<GpsTag> gpsTags() const;

    bool contains(ImageTag tag) const;
    bool contains(ExtendedTag tag) const;
    bool contains(GpsTag tag) const;

    void remove(ImageTag tag);
    void remove(ExtendedTag tag);
    void remove(GpsTag tag);

    ExifValue value(ImageTag tag) const;
    ExifValue value(ExtendedTag tag) const;
    ExifValue value(GpsTag tag) const;

    void setValue(ImageTag tag, const ExifValue &value);
    void setValue(ExtendedTag tag, const ExifValue &value);
    void setValue(GpsTag tag, const ExifValue &value);

    QImage thumbnail() const;
    void setThumbnail(const QImage &thumbnail);
	
private:	
	const ExifIfd exifIFD() const;
	const ExifIfd gpsIFD() const;
	ExifIfd &exifIFD();
	ExifIfd &gpsIFD();
	
	void loadFromJpeg(QDataStream &fileStream); // throw(ExifException)
	void saveToJpeg(QDataStream &fileStream) const; // throw(ExifException)
	void saveToJpeg(ExifMarker app1) const; // throw(ExifException)
	
	typedef QMap<int, ExifIfd> ExifIfdMap;
	ExifIfdMap ifds;
	
	QDataStream::ByteOrder byteOrder;
	static const QByteArray exifHeader;
};

#endif // EXIFIMAGEHEADER_H
