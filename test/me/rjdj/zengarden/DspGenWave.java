/*
 *  Copyright 2012 Reality Jockey, Ltd.
 *                 info@rjdj.me
 *                 http://rjdj.me/
 * 
 *  This file is part of ZenGarden.
 *
 *  ZenGarden is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ZenGarden is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ZenGarden.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

package me.rjdj.zengarden;

import java.io.File;

import javax.sound.sampled.AudioFileFormat;
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

public class DspGenWave extends ZenGardenAdapter {

  private static final int BLOCK_SIZE = 64;
  private static final int NUM_INPUT_CHANNELS = 1;
  private static final int NUM_OUTPUT_CHANNELS = 2;
  private static final float SAMPLE_RATE = 44100.0f;

  /**
   * Generates a wave file based on the given Pd file for the given duration. To be used for
   * generating golden wavs.
   */
  public static void main(String[] args) throws Exception {
    if (args.length < 2) {
      System.out.println("Usage: DspGenWave path/to/pd/file durationInMs\n" +
      		"  -in/--input path/to/input.wav");
      return;
    }
    
    File pdFile = new File(args[0]).getCanonicalFile();
    File wavFile = new File(pdFile.getPath().split("\\.")[0] + ".golden.wav");
    float durationMs = new Float(args[1]).floatValue();
    
    AudioInputStream ais = null;
    if (args.length > 2) {
      if ("-in".equals(args[2]) || "--input".equals(args[2])) {
        File inputFile = new File(args[3]).getCanonicalFile();
        ais = AudioSystem.getAudioInputStream(inputFile);
        if (ais.getFormat().getChannels() != NUM_INPUT_CHANNELS) {
          throw new RuntimeException("The input audio file must have the same number of channels as ZenGarden: " +
              ais.getFormat().getChannels() + " != " + NUM_INPUT_CHANNELS);
        }
        if (ais.getFormat().getSampleRate() != SAMPLE_RATE) {
          throw new RuntimeException("Input audio file sample rate does not match.");
        }
      }
    }

    // 16-bit signed mono 
    AudioFormat audioFormat = new AudioFormat(SAMPLE_RATE, 16, NUM_OUTPUT_CHANNELS, true, false);
    ZGInputStream zgis = new ZGInputStream(pdFile, NUM_INPUT_CHANNELS, NUM_OUTPUT_CHANNELS,
        BLOCK_SIZE, SAMPLE_RATE, ais);
    
    AudioInputStream inputStream = new AudioInputStream(zgis, audioFormat,
        (long) (SAMPLE_RATE*durationMs/1000.0f));
    AudioSystem.write(inputStream, AudioFileFormat.Type.WAVE, wavFile);
  }
}
