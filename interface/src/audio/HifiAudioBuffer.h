#ifndef HIFIAUDIOBUFFER_H
#define HIFIAUDIOBUFFER_H

#include <QObject>
#include <qqml.h>
#include <QAudioOutput>
#include <QBuffer>
#include <QByteArray>


class BufferIODevice: public QIODevice {
    Q_OBJECT
public:
    BufferIODevice(qint64 bufferSize);
    void start();
    void stop();
    qint64 writeData(const char *data, qint64 len) override;
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 remainingBuffer();
    qint64 bytesAvailable() const override;
    bool isPlaying();
private:
    qint64 m_bufferSize;
    qint64 m_availableBytes;
    qint64 m_remainingBytes;
    qint64 m_writePos;
    qint64 m_readPos;
    QByteArray m_buffer;
signals:
    void bufferEmpty();
};



/*@jsdoc
 * A <code>HifiAudioBuffer</code> plays audio buffers from memory inside Interface.
 *
 * <p>It allows you to play sound generated dynamically by JavaScript.
 * The sound is played locally, not transmitted over the Internet.</p>
 *
 * <p>Create using <code>new HifiAudioBuffer(...)</code>.</p>
 *
 * @class HifiAudioBuffer
 * @param {number} [bufferSize] - Size of the audio buffer stored in the memory.
 * @param {HifiAudioBuffer.Properties} [properties] - Describes type of the data in the buffer.
 *
 * @hifi-interface
 * @hifi-client-entity
 * @hifi-avatar
 *
 * @property {bool} playing - State of the buffer playback.
 * @property {number} volume - Audio volume between 0.0 and 1.0.
 */

class HifiAudioBuffer : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool playing READ isPlaying WRITE setPlaying NOTIFY playingChanged)
    Q_PROPERTY(qreal volume READ getVolume WRITE setVolume NOTIFY volumeChanged)

    Q_PROPERTY(qint32 channels READ getChannels WRITE setChannels)
    Q_PROPERTY(qint32 sampleRate READ getSampleRate WRITE setSampleRate)
    Q_PROPERTY(qint32 sampleSize READ getSampleSize WRITE setSampleSize)
    Q_PROPERTY(SampleType sampleType READ getSampleType WRITE setSampleType)
    Q_PROPERTY(ByteOrder byteOrder READ getByteOrder WRITE setByteOrder)
    Q_PROPERTY(qint64 bufferSize READ getBufferSize WRITE setBufferSize)

private:
    QScopedPointer<BufferIODevice> m_bufferDevice;

    QScopedPointer<QAudioOutput> m_audioOutput;

    QAudioFormat m_audioFormat;

    qint64 m_bufferSize;

public:
    enum SampleType { Int = 0, UnsignedInt = 1, Float = 2 };
    enum ByteOrder { LittleEndian = 0, BigEndian = 1 };

    Q_ENUM(ByteOrder);
    Q_ENUM(SampleType);

    explicit HifiAudioBuffer(QObject *parent = nullptr);

    ~HifiAudioBuffer();

    Q_INVOKABLE void initAudioDevice();

    /*@jsdoc
     * @function HifiAudioBuffer.remainingBuffer
     * @returns {number} Returns the difference in the number of bytes written
     * to the buffer and read by the audio device. Calling this method resets the counter.
     */
    Q_INVOKABLE qint64 remainingBuffer();

    /*@jsdoc
     * @function HifiAudioBuffer.write
     * @param {ArrayBuffer} data - binary data that will be appended to the buffer.
     * @example <caption>Example of the audio generation.</caption>
     * // JavaScript file.
     *
     * var params = {
     *    channels: 2,
     *    sampleRate: 44100,
     *    sampleSize: 32,
     *    sampleType: "float"
     * };
     *
     * var data = generateData(1000, params, 600); // 1 second, 600 Hz
     * var audioBuffer = new HifiAudioBuffer(data.byteLength, params);
     * audioBuffer.bufferEmpty.connect(stopAudio);
     * audioBuffer.write(data.buffer);
     * audioBuffer.playing = true;  // start audio
     *
     * function stopAudio() {
     *    audioBuffer.bufferEmpty.disconnect(stopAudio);
     *    audioBuffer.playing = false;  // stop audio
     * }
     *
     * function generateData(durationMs, format, freqHz) {
     *    var length = Math.floor(format.sampleRate * format.channels * durationMs / 1000);
     *    var data = new Float32Array(length);
     *    var sampleIndex = 0;
     *    var i, x, pos = 0;
     *
     *    while (pos < length) {
     *      x = Math.sin(2 * Math.PI * freqHz * (sampleIndex % format.sampleRate) / format.sampleRate);
     *      for (i = 0; i < format.channels; i++) {
     *          data[pos++] = x;
     *      }
     *      sampleIndex++;
     *    }
     *    return data;
     * }
     */
    Q_INVOKABLE void write(const QByteArray& data);

public slots:

    qint32 getChannels();
    qint32 getSampleRate();
    qint32 getSampleSize();
    SampleType getSampleType();
    ByteOrder getByteOrder();
    qint64 getBufferSize();

    void setChannels(const qint32 channels);
    void setSampleRate(const qint32 sampleRate);
    void setSampleSize(const qint32 sampleSize);
    void setSampleType(const SampleType sampleType);
    void setByteOrder(const ByteOrder byteOrder);
    void setBufferSize(const qint64 bufferSize);

    /*@jsdoc
     * Gets whether the audio buffer is playing.
     * @function HifiAudioBuffer.isPlaying
     * @returns {boolean} <code>true</code> if the audio is playing, <code>false</code> if it is stopped.
     */
    bool isPlaying();

    /*@jsdoc
     * Starts or stops audio playback.
     * @function HifiAudioBuffer.setPlaying
     * @param {boolean} playing - <code>true</code> to start the audio, <code>false</code> to stop it.
     */
    void setPlaying(bool playing);

    /*@jsdoc
     * Gets current audio volume.
     * @function HifiAudioBuffer.getVolume
     * @returns {number} Current audio volume between 0.0 and 1.0.
     */
    qreal getVolume();

    /*@jsdoc
     * Changes audio volume.
     * @function HifiAudioBuffer.setVolume
     * @param {number} volume - audio volume between 0.0 and 1.0.
     */
    void setVolume(qreal volume);

    void sendBufferEmpty();

signals:

    /*@jsdoc
     * Triggered when the audio playback has changed.
     * @function HifiAudioBuffer.playingChanged
     * @param {boolean} playing - <code>true</code> when audio is playing, <code>false</code> otherwise.
     * @returns {Signal}
     */
    void playingChanged(bool playing);

    /*@jsdoc
     * Triggered when the audio volume has changed.
     * @function HifiAudioBuffer.volumeChanged
     * @param {number} volume - new audio volume between 0.0 and 1.0.
     * @returns {Signal}
     */
    void volumeChanged(qreal volume);

    /*@jsdoc
     * Triggered when audio device is trying to buffer buffer is empty.
     * @function HifiAudioBuffer.bufferEmpty
     * @returns {Signal}
     */
    void bufferEmpty();
};

#endif // HIFIAUDIOBUFFER_H
