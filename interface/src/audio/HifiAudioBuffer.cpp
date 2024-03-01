#include "HifiAudioBuffer.h"

#include <qendian.h>
#include "AudioClient.h"

static const qint64 MIN_BUFFER_SIZE = 44100;

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


HifiAudioBuffer::HifiAudioBuffer(QObject *parent)
    : QObject{parent}
{
    m_audioFormat.setSampleRate(44100);
    m_audioFormat.setChannelCount(1);
    m_audioFormat.setSampleSize(32);
    m_audioFormat.setCodec("audio/pcm");
    m_audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    m_audioFormat.setSampleType(QAudioFormat::Float);
    m_bufferSize = MIN_BUFFER_SIZE;
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
void HifiAudioBuffer::initAudioDevice() {
    auto client = DependencyManager::get<AudioClient>().data();
    const QAudioDeviceInfo &deviceInfo = client->getActiveAudioDevice(QAudio::AudioOutput).getDevice();

    if (!deviceInfo.isFormatSupported(m_audioFormat)) {
        qDebug() << "HifiAudioBuffer::initAudioOutput: Audio format not supported";
        m_audioFormat = deviceInfo.nearestFormat(m_audioFormat);
    }

    if (m_bufferSize < MIN_BUFFER_SIZE) {
        m_bufferSize = MIN_BUFFER_SIZE;
    }

    m_audioOutput.reset(new QAudioOutput(deviceInfo, m_audioFormat));
    m_bufferDevice.reset(new BufferIODevice(m_bufferSize));
}


HifiAudioBuffer::~HifiAudioBuffer() {
    setPlaying(false);
}


void HifiAudioBuffer::setPlaying(bool playing) {
    if (!m_bufferDevice || !m_audioOutput) {
      return;
    }
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
    if (!m_bufferDevice) {
      return false;
    }
    return m_bufferDevice->isPlaying();
}

void HifiAudioBuffer::setVolume(const qreal volume) {
    qreal newVolume = volume;
    if (!m_audioOutput) {
      return;
    }
    if (newVolume < 0.0) {
      newVolume = 0.0;
    } else if (newVolume >= 1.0) {
      newVolume = 1.0;
    }
    m_audioOutput->setVolume(newVolume);
    emit volumeChanged(newVolume);
}

qreal HifiAudioBuffer::getVolume() {
    if (!m_audioOutput) {
      return 0.0;
    }
    return m_audioOutput->volume();
}

qint64 HifiAudioBuffer::remainingBuffer() {
    if (!m_bufferDevice) {
      return 0;
    }
    return m_bufferDevice->remainingBuffer();
}

void HifiAudioBuffer::write(const QByteArray& data) {
    if (!m_bufferDevice) {
      return;
    }
    m_bufferDevice->writeData(data.constData(), data.size());
}

void HifiAudioBuffer::sendBufferEmpty() {
    emit bufferEmpty();
}

qint32 HifiAudioBuffer::getChannels() {
    return m_audioFormat.channelCount();
}

qint32 HifiAudioBuffer::getSampleRate() {
    return m_audioFormat.sampleRate();
}

qint32 HifiAudioBuffer::getSampleSize() {
    return m_audioFormat.sampleSize();
}

HifiAudioBuffer::SampleType HifiAudioBuffer::getSampleType() {
    switch (m_audioFormat.sampleType()) {
        case QAudioFormat::SignedInt:
            return SampleType::Int;
        case QAudioFormat::UnSignedInt:
            return SampleType::UnsignedInt;
        case QAudioFormat::Float:
            return SampleType::Float;
        case QAudioFormat::Unknown:
            return SampleType::Float;
    }
    return SampleType::Float;
}

HifiAudioBuffer::ByteOrder HifiAudioBuffer::getByteOrder() {

    switch (m_audioFormat.byteOrder()) {
        case QAudioFormat::LittleEndian:
            return ByteOrder::LittleEndian;
        case QAudioFormat::BigEndian:
            return ByteOrder::BigEndian;
    }
    return ByteOrder::LittleEndian;
}

qint64 HifiAudioBuffer::getBufferSize() {
    return m_bufferSize;
}

void HifiAudioBuffer::setChannels(const qint32 channels) {
    m_audioFormat.setChannelCount(channels);
}

void HifiAudioBuffer::setSampleRate(const qint32 sampleRate) {
    m_audioFormat.setSampleRate(sampleRate);
}

void HifiAudioBuffer::setSampleSize(const qint32 sampleSize) {
    m_audioFormat.setSampleSize(sampleSize);
}

void HifiAudioBuffer::setSampleType(const SampleType sampleType) {
    switch (sampleType) {
        case SampleType::Int:
            m_audioFormat.setSampleType(QAudioFormat::SignedInt);
            break;
        case SampleType::UnsignedInt:
            m_audioFormat.setSampleType(QAudioFormat::UnSignedInt);
            break;
        case SampleType::Float:
            m_audioFormat.setSampleType(QAudioFormat::Float);
            break;
    }
}

void HifiAudioBuffer::setByteOrder(const ByteOrder byteOrder) {
    switch (byteOrder) {
        case ByteOrder::LittleEndian:
            m_audioFormat.setByteOrder(QAudioFormat::LittleEndian);
            break;
        case ByteOrder::BigEndian:
            m_audioFormat.setByteOrder(QAudioFormat::BigEndian);
            break;
    }
}

void HifiAudioBuffer::setBufferSize(const qint64 bufferSize) {
    m_bufferSize = bufferSize;
}
