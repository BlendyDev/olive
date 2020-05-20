/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2019 Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef RENDERWORKER_H
#define RENDERWORKER_H

#include <QMatrix4x4>

#include "decodercache.h"
#include "node/traverser.h"
#include "node/output/viewer/viewer.h"

OLIVE_NAMESPACE_ENTER

class RenderBackend;

class RenderWorker : public QObject, public NodeTraverser
{
  Q_OBJECT
public:
  RenderWorker(RenderBackend* parent);

  virtual ~RenderWorker() override;

  void Init(ViewerOutput *viewer);
  void Close();

  void Queue(NodeInput* input);
  void ProcessQueue();

  bool IsAvailable() const
  {
    return available_;
  }

  void SetAvailable(bool a)
  {
    available_ = a;
  }

  ViewerOutput* GetViewer() const
  {
    return viewer_;
  }

  void SetVideoParams(const VideoRenderingParams& params)
  {
    video_params_ = params;
  }

  void SetAudioParams(const AudioRenderingParams& params)
  {
    audio_params_ = params;
  }

  void SetVideoDownloadMatrix(const QMatrix4x4& mat)
  {
    video_download_matrix_ = mat;
  }

  void SetAudioModeIsPreview(bool audio_mode_is_preview)
  {
    audio_mode_is_preview_ = audio_mode_is_preview;
  }

  /**
   * @brief Return a unique ID for the image generated at this time
   *
   * This hash should always be unique to this image and can therefore be used to match existing
   * cached frames.
   *
   * @return
   *
   * SHA-1 hash or empty QByteArray if no viewer node is set.
   */
  QByteArray Hash(const rational &time, bool block_for_update);

  /**
   * @brief Render the frame at this time
   *
   * Produces a fully rendered frame from the connected viewer at this time.
   *
   * @return
   *
   * A frame corresponding to the set video parameters. If no nodes are active at the time, this
   * function will still return a blank frame with the same parameters. If no viewer node is set,
   * nullptr is returned.
   */
  FramePtr RenderFrame(const rational& time, bool block_for_update);

  SampleBufferPtr RenderAudio(const TimeRange& range, bool block_for_update);

protected:
  virtual void TextureToFrame(const QVariant& texture, FramePtr frame, const QMatrix4x4 &mat) const = 0;

  virtual NodeValue FrameToTexture(DecoderPtr decoder, StreamPtr stream, const TimeRange &range) const = 0;

  virtual void FootageProcessingEvent(StreamPtr stream, const TimeRange &input_time, NodeValueTable* table) override;

  virtual NodeValueTable GenerateBlockTable(const TrackOutput *track, const TimeRange &range) override;

  virtual void ProcessNodeEvent(const Node *node, const TimeRange &range, NodeValueDatabase &input_params_in, NodeValueTable &output_params) override;

  const VideoRenderingParams& video_params() const
  {
    return video_params_;
  }

  const AudioRenderingParams& audio_params() const
  {
    return audio_params_;
  }

signals:
  void AudioConformUnavailable(StreamPtr stream, TimeRange range,
                               rational stream_time, AudioRenderingParams params);

  void FinishedJob();

private:
  void UpdateData(bool block_for_update);

  NodeValue GetDataFromStream(StreamPtr stream, const TimeRange& input_time);

  DecoderPtr ResolveDecoderFromInput(StreamPtr stream);

  void CopyNodeInputValue(NodeInput* input);
  Node *CopyNodeConnections(Node *src_node);
  void CopyNodeMakeConnection(NodeInput *src_input, NodeInput *dst_input);

  RenderBackend* parent_;

  VideoRenderingParams video_params_;

  AudioRenderingParams audio_params_;

  struct CachedStill {
    NodeValue texture;
    QString colorspace;
    bool alpha_is_associated;
    int divider;
    rational time;
  };

  QHash<Stream*, CachedStill> still_image_cache_;

  QMatrix4x4 video_download_matrix_;

  DecoderCache decoder_cache_;

  ViewerOutput* viewer_;
  TimeRange audio_render_time_;
  QList<NodeInput*> queued_updates_;
  QHash<Node*, Node*> copy_map_;
  bool available_;

  bool audio_mode_is_preview_;

  QMutex lock_;

};

OLIVE_NAMESPACE_EXIT

#endif // RENDERWORKER_H
