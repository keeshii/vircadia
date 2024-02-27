#ifndef HIFIAUDIOBUFFER_H
#define HIFIAUDIOBUFFER_H

#include <QObject>
#include <qqml.h>
#include <QAudioOutput>
#include <QBuffer>
#include <QByteArray>

#include <QtScript/QScriptValue>
#include <QtScript/QScriptContext>


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


private:
    static QScriptValue internal_constructor(QScriptContext* context, QScriptEngine* engine, bool restricted);

    void initAudioOutput(const QScriptContext* context);

    QScopedPointer<BufferIODevice> m_bufferDevice;

    QScopedPointer<QAudioOutput> m_audioOutput;

    const bool _restricted;

public:
    static QScriptValue constructor(QScriptContext* context, QScriptEngine* engine) {
        return internal_constructor(context, engine, false);
    }

    static QScriptValue restricted_constructor(QScriptContext* context, QScriptEngine* engine ){
        return internal_constructor(context, engine, true);
    }

    HifiAudioBuffer(bool restricted);
    ~HifiAudioBuffer();

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
