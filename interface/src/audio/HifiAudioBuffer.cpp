#include "HifiAudioBuffer.h"

#include <qendian.h>

#include "AudioClient.h"
#include <QtScript/QScriptContext>
#include <QtScript/QScriptEngine>

static const qint64 MIN_BUFFER_SIZE = 44100;
static const char* const CHANNELS_PROPERTY = "channels";
static const char* const SAMPLE_RATE_PROPERTY = "sampleRate";
static const char* const SAMPLE_SIZE_PROPERTY = "sampleSize";
static const char* const BYTE_ORDER_PROPERTY = "byteOrder";
static const char* const BYTE_ORDER_BIG_ENDIAN = "big";
static const char* const SAMPLE_TYPE_PROPERTY = "sampleType";
static const char* const SAMPLE_TYPE_INT = "int";
static const char* const SAMPLE_TYPE_UINT = "uint";
static const char* const SAMPLE_TYPE_FLOAT = "float";


BufferIODevice::BufferIODevice(qint64 bufferSize) {
    m_bufferSize = bufferSize;
    m_writePos = 0;
    m_readPos = 0;
    m_availableBytes = 0;
    m_remainingBytes = 0;
    m_buffer.resize(m_bufferSize);
}


void BufferIODevice::start() {
    open(QIODevice::ReadOnly);
}

void BufferIODevice::stop() {
    m_readPos = 0;
    m_writePos = 0;
    m_availableBytes = 0;
    m_remainingBytes = 0;
    close();
}

qint64 BufferIODevice::writeData(const char *data, qint64 len) {
    qint64 total = 0;

    m_remainingBytes += len;
    m_availableBytes += len;

    if (!m_buffer.isEmpty()) {
        while (total < len) {
            const qint64 chunk = qMin((m_buffer.size() - m_writePos), len - total);
            memcpy(m_buffer.data() + m_writePos, data + total, chunk);
            m_writePos = (m_writePos + chunk) % m_buffer.size();
            total += chunk;
        }
    }
    return total;
}

qint64 BufferIODevice::readData(char *data, qint64 len) {
    qint64 total = 0;

    if (m_availableBytes == 0) {
        memset(data, 0, len);
        total = len;
        return total;
    }

    m_remainingBytes -= len;
    m_availableBytes = (m_availableBytes > len ? m_availableBytes - len : 0);

    if (!m_buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((m_buffer.size() - m_readPos), len - total);
            memcpy(data + total, m_buffer.constData() + m_readPos, chunk);
            m_readPos = (m_readPos + chunk) % m_buffer.size();
            total += chunk;
        }
    }

    if (m_availableBytes == 0) {
        emit bufferEmpty();
    }

    return total;
}

qint64 BufferIODevice::bytesAvailable() const {
    return m_availableBytes;
}

qint64 BufferIODevice::remainingBuffer() {
    qint64 remainingBytes = m_remainingBytes;
    m_remainingBytes = 0;
    return remainingBytes;
}

bool BufferIODevice::isPlaying() {
    return isOpen();
}

// Method called by Qt scripts to create a new web window in the overlay
QScriptValue HifiAudioBuffer::internal_constructor(QScriptContext* context, QScriptEngine* engine, bool restricted) {
    HifiAudioBuffer* retVal = new HifiAudioBuffer(restricted);
    Q_ASSERT(retVal);

    retVal->initAudioOutput(context);
    Q_ASSERT(retVal->m_audioOutput);
    Q_ASSERT(retVal->m_bufferDevice);

    connect(retVal->m_bufferDevice.data(), &BufferIODevice::bufferEmpty, retVal, &HifiAudioBuffer::sendBufferEmpty);
    connect(engine, &QScriptEngine::destroyed, retVal, &HifiAudioBuffer::deleteLater);

    return engine->newQObject(retVal);
}


HifiAudioBuffer::HifiAudioBuffer(bool restricted) : _restricted(restricted) {
}


/*@jsdoc
 * Properties used to initialize an {@link HifiAudioBuffer}.
 * @typedef {object} HifiAudioBuffer.Properties
 * @property {number} [channels=1] - Number of audio channels.
 * @property {number} [sampleRate=44100] - Samples per second.
 * @property {number} [sampleSize=32] - Size of the audio sample per channel.
 * @property {string} [byteOrder="little"] - Byte order of the sample.
 * Possible values: <code>little</code> - little endian, <code>big</code> - big endian.
 * @property {string} [sampleType="float"] Data format.
 * Possible values: <code>int</code>, <code>uint</code> and <code>float</code>.
 */
void HifiAudioBuffer::initAudioOutput(const QScriptContext* context) {
    const auto argumentCount = context->argumentCount();

    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);

    qint64 bufferSize = MIN_BUFFER_SIZE;
    if (argumentCount >= 1 && !context->argument(0).isUndefined()) {
        bufferSize = context->argument(0).toInt32();
    }

    if (argumentCount >= 2 && !context->argument(1).isUndefined()) {
        const QVariantMap properties = context->argument(1).toVariant().toMap();
        if (!properties[SAMPLE_RATE_PROPERTY].isNull()) {
            format.setSampleRate(properties[SAMPLE_RATE_PROPERTY].toUInt());
        }
        if (!properties[CHANNELS_PROPERTY].isNull()) {
            format.setChannelCount(properties[CHANNELS_PROPERTY].toUInt());
        }
        if (!properties[SAMPLE_SIZE_PROPERTY].isNull()) {
            format.setSampleSize(properties[SAMPLE_SIZE_PROPERTY].toUInt());
        }
        if (!properties[BYTE_ORDER_PROPERTY].isNull()) {
            const QString byteOrder = properties[BYTE_ORDER_PROPERTY].toString();
            if (QString::compare(byteOrder, BYTE_ORDER_BIG_ENDIAN, Qt::CaseInsensitive) == 0) {
                format.setByteOrder(QAudioFormat::BigEndian);
            }
        }
        if (!properties[SAMPLE_TYPE_PROPERTY].isNull()) {
            const QString sampleType = properties[SAMPLE_TYPE_PROPERTY].toString();
            if (QString::compare(sampleType, SAMPLE_TYPE_INT, Qt::CaseInsensitive) == 0) {
                format.setSampleType(QAudioFormat::SignedInt);
            } else if (QString::compare(sampleType, SAMPLE_TYPE_UINT, Qt::CaseInsensitive) == 0) {
                format.setSampleType(QAudioFormat::UnSignedInt);
            } else if (QString::compare(sampleType, SAMPLE_TYPE_FLOAT, Qt::CaseInsensitive) == 0) {
                format.setSampleType(QAudioFormat::Float);
            } else {
                format.setSampleType(QAudioFormat::Unknown);
            }
        }
    }

    auto client = DependencyManager::get<AudioClient>().data();
    const QAudioDeviceInfo &deviceInfo = client->getActiveAudioDevice(QAudio::AudioOutput).getDevice();

    if (bufferSize < MIN_BUFFER_SIZE) {
      bufferSize = MIN_BUFFER_SIZE;
      qDebug() << "HifiAudioBuffer::initAudioOutput: incorrect buffer size, fallback to default";
    }

    if (!deviceInfo.isFormatSupported(format)) {
        qDebug() << "HifiAudioBuffer::initAudioOutput: Audio format not supported";
        format = deviceInfo.nearestFormat(format);
    }

    m_audioOutput.reset(new QAudioOutput(deviceInfo, format));
    m_bufferDevice.reset(new BufferIODevice(bufferSize));
}


HifiAudioBuffer::~HifiAudioBuffer() {
    setPlaying(false);
}


void HifiAudioBuffer::setPlaying(bool playing) {
    if (playing) {
        m_bufferDevice->start();
        m_audioOutput->start(m_bufferDevice.data());
    } else {
        m_audioOutput->stop();
        m_bufferDevice->stop();
    }
    emit playingChanged(playing);
}


bool HifiAudioBuffer::isPlaying() {
    return m_bufferDevice->isPlaying();
}

void HifiAudioBuffer::setVolume(const qreal volume) {
    qreal newVolume = volume;
    if (newVolume < 0.0) {
      newVolume = 0.0;
    } else if (newVolume >= 1.0) {
      newVolume = 1.0;
    }
    m_audioOutput->setVolume(newVolume);
    emit volumeChanged(newVolume);
}

qreal HifiAudioBuffer::getVolume() {
    return m_audioOutput->volume();
}

qint64 HifiAudioBuffer::remainingBuffer() {
    return m_bufferDevice->remainingBuffer();
}

void HifiAudioBuffer::write(const QByteArray& data) {
    m_bufferDevice->writeData(data.constData(), data.size());
}

void HifiAudioBuffer::sendBufferEmpty() {
    emit bufferEmpty();
}
