# MyBe ( Make Your Best Effort )
Experiment to recode HLS Transport segments to improve playback on network glitches.

Live HLS streams can be seriously impacted when a segment doesn't download fast enough.
In ABR scenarios, the playback may also drop the bitrate.

Results can be lost audio, stalled imagery, a drop in bitrate that takes a while to 
recover even if it was only a short term network drop.

Worse, it is not uncommon for an adaptive player to drop a segment that even might arrive
late, and use the rest of the timeslot to load a lower bitrate version. Since a large
chunk of the time slot has already been wasted, this will force a much lower bitrate
selection than wanted.

This experiment will recode the transport segment.

 . Reorganize the transport packets such that everything except
   video is placed at the start, and the video stream is placed 
   at the end.

 . Code a new video stream using the same initial I-Frame but
   fewer frames ( same quality ).

 . Insert some custom metadata ( where TBD ) that signals the number
   of video frames, PCR/PTS spread etc.

 . Insert a new stream on a different PID after the initial Iframe
   for the new video substream. This substream should have the same
   PCR, transport stream CC values etc. So that it mirrors the original.

The result should be playable on most software demuxer based players and many hardware
ones. Hardware decoders with limited rate buffers may not be able to handle the new
stream because the interleaving will of changed, giving a short burst to the bitrate
required for non video streams.

A suitable segment loader however, will be capable of delivering something other than
no-data or all-data based on a timeout.

Once the loader hits a specified timeout value, it can make a decision on what to deliver
to the client.

 . If the loader did not get everything up to the custom metadata - fail/timeout to client.

 . If the loader got as far as the custom data - it can reproduce a segment to return which
   will have valid audio, subtitles etc. but an empty video. ( maybe returning empty P frames
   such that existing video would not change? ).

 . If the loader got as far as the initial I-Frame, loader will reproduce a segment with
   audio etc. the initial I-Frame, and a video stream of empty P frames ).

 . If the loader got the newly coded video data. A segment is reproduced with the lower
   framerate video.

 . If the loader got everything, then original video is returned.

The loader will also remux the packets based on DTS and PCR in order to get as close as
possible to the original video.

## Expected Result

The expectation is that in scenarios where a temporary network glitch causes a segment to
be delayed, something less jarring will be experienced by the end user.

The player may elect to change bitrate, or simply note the delay and use this to track
bitrate for a potential later change.



