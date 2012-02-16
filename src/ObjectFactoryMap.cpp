/*
 *  Copyright 2011 Reality Jockey, Ltd.
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

#include "ObjectFactoryMap.h"

// all standard message objects
#include "MessageAbsoluteValue.h"
#include "MessageAdd.h"
#include "MessageArcTangent.h"
#include "MessageArcTangent2.h"
#include "MessageBang.h"
#include "MessageCosine.h"
#include "MessageCputime.h"
#include "MessageChange.h"
#include "MessageClip.h"
#include "MessageCputime.h"
#include "MessageDeclare.h"
#include "MessageDelay.h"
#include "MessageDivide.h"
#include "MessageDbToPow.h"
#include "MessageDbToRms.h"
#include "MessageEqualsEquals.h"
#include "MessageExp.h"
#include "MessageFloat.h"
#include "MessageFrequencyToMidi.h"
#include "MessageGreaterThan.h"
#include "MessageGreaterThanOrEqualTo.h"
#include "MessageInlet.h"
#include "MessageInteger.h"
#include "MessageLessThan.h"
#include "MessageLessThanOrEqualTo.h"
#include "MessageLine.h"
#include "MessageListAppend.h"
#include "MessageListLength.h"
#include "MessageListPrepend.h"
#include "MessageListSplit.h"
#include "MessageListTrim.h"
#include "MessageLoadbang.h"
#include "MessageLog.h"
#include "MessageLogicalAnd.h"
#include "MessageLogicalOr.h"
#include "MessageMaximum.h"
#include "MessageMessageBox.h"
#include "MessageMetro.h"
#include "MessageMidiToFrequency.h"
#include "MessageMinimum.h"
#include "MessageModulus.h"
#include "MessageMoses.h"
#include "MessageMultiply.h"
#include "MessageNotEquals.h"
#include "MessageNotein.h"
#include "MessageOpenPanel.h"
#include "MessageOutlet.h"
#include "MessagePack.h"
#include "MessagePipe.h"
#include "MessagePow.h"
#include "MessagePowToDb.h"
#include "MessagePrint.h"
#include "MessageRandom.h"
#include "MessageReceive.h"
#include "MessageRemainder.h"
#include "MessageRmsToDb.h"
#include "MessageRoute.h"
#include "MessageSamplerate.h"
#include "MessageSelect.h"
#include "MessageSend.h"
#include "MessageSine.h"
#include "MessageSoundfiler.h"
#include "MessageSpigot.h"
#include "MessageSqrt.h"
#include "MessageStripNote.h"
#include "MessageSubtract.h"
#include "MessageSwitch.h"
#include "MessageSwap.h"
#include "MessageSymbol.h"
#include "MessageTable.h"
#include "MessageTableRead.h"
#include "MessageTableWrite.h"
#include "MessageTangent.h"
#include "MessageText.h"
#include "MessageTimer.h"
#include "MessageToggle.h"
#include "MessageTrigger.h"
#include "MessageUntil.h"
#include "MessageUnpack.h"
#include "MessageValue.h"
#include "MessageWrap.h"

// all standard dsp objects
#include "DspAdc.h"
#include "DspAdd.h"
#include "DspBandpassFilter.h"
#include "DspBang.h"
#include "DspCatch.h"
#include "DspClip.h"
#include "DspCosine.h"
#include "DspDac.h"
#include "DspDelayRead.h"
#include "DspDelayWrite.h"
#include "DspDivide.h"
#include "DspEnvelope.h"
#include "DspHighpassFilter.h"
#include "DspInlet.h"
#include "DspLine.h"
#include "DspLog.h"
#include "DspLowpassFilter.h"
#include "DspMinimum.h"
#include "DspMultiply.h"
#include "DspNoise.h"
#include "DspOsc.h"
#include "DspOutlet.h"
#include "DspPhasor.h"
#include "DspPrint.h"
#include "DspReceive.h"
#include "DspReciprocalSqrt.h"
#include "DspRfft.h"
#include "DspRifft.h"
#include "DspSend.h"
#include "DspSignal.h"
#include "DspSqrt.h"
#include "DspSnapshot.h"
#include "DspSubtract.h"
#include "DspTablePlay.h"
#include "DspTableRead.h"
#include "DspTableRead4.h"
#include "DspThrow.h"
#include "DspVariableDelay.h"
#include "DspVCF.h"
#include "DspWrap.h"

ObjectFactoryMap::ObjectFactoryMap() {
  // these objects represent the core set of supported objects
  
  // message objects
  objectFactoryMap[string(MessageAbsoluteValue::getObjectLabel())] = &MessageAbsoluteValue::newObject;
  objectFactoryMap[string(MessageAdd::getObjectLabel())] = &MessageAdd::newObject;
  objectFactoryMap[string(MessageArcTangent::getObjectLabel())] = &MessageArcTangent::newObject;
  objectFactoryMap[string(MessageArcTangent2::getObjectLabel())] = &MessageArcTangent2::newObject;
  objectFactoryMap[string(MessageBang::getObjectLabel())] = &MessageBang::newObject;
  objectFactoryMap[string("bng")] = &MessageBang::newObject;
  objectFactoryMap[string("b")] = &MessageBang::newObject;
  objectFactoryMap[string(MessageChange::getObjectLabel())] = &MessageChange::newObject;
  objectFactoryMap[string(MessageClip::getObjectLabel())] = &MessageClip::newObject;
  objectFactoryMap[string(MessageCosine::getObjectLabel())] = &MessageCosine::newObject;
  objectFactoryMap[string(MessageCputime::getObjectLabel())] = &MessageCputime::newObject;
  objectFactoryMap[string(MessageDbToPow::getObjectLabel())] = &MessageDbToPow::newObject;
  objectFactoryMap[string(MessageDbToRms::getObjectLabel())] = &MessageDbToRms::newObject;
  objectFactoryMap[string(MessageDeclare::getObjectLabel())] = &MessageDeclare::newObject;
  objectFactoryMap[string(MessageDelay::getObjectLabel())] = &MessageDelay::newObject;
  objectFactoryMap[string("del")] = &MessageDelay::newObject;
  objectFactoryMap[string(MessageDivide::getObjectLabel())] = &MessageDivide::newObject;
  objectFactoryMap[string(MessageEqualsEquals::getObjectLabel())] = &MessageEqualsEquals::newObject;
  objectFactoryMap[string(MessageExp::getObjectLabel())] = &MessageExp::newObject;
  objectFactoryMap[string(MessageFloat::getObjectLabel())] = &MessageFloat::newObject;
  objectFactoryMap[string("f")] = &MessageFloat::newObject;
  objectFactoryMap[string("nbx")] = &MessageFloat::newObject; // number boxes are represented as float objects
  objectFactoryMap[string("hsl")] = &MessageFloat::newObject; // horizontal and vertical sliders are
  objectFactoryMap[string("vsl")] = &MessageFloat::newObject; // represened as float boxes
  objectFactoryMap[string(MessageFrequencyToMidi::getObjectLabel())] = &MessageFrequencyToMidi::newObject;
  objectFactoryMap[string(MessageGreaterThan::getObjectLabel())] = &MessageGreaterThan::newObject;
  objectFactoryMap[string(MessageGreaterThanOrEqualTo::getObjectLabel())] = &MessageGreaterThanOrEqualTo::newObject;
  objectFactoryMap[string(MessageInlet::getObjectLabel())] = &MessageInlet::newObject;
  objectFactoryMap[string(MessageInteger::getObjectLabel())] = &MessageInteger::newObject;
  objectFactoryMap[string("i")] = &MessageInteger::newObject;
  objectFactoryMap[string(MessageLessThan::getObjectLabel())] = &MessageLessThan::newObject;
  objectFactoryMap[string(MessageLessThanOrEqualTo::getObjectLabel())] = &MessageLessThanOrEqualTo::newObject;
  objectFactoryMap[string(MessageLine::getObjectLabel())] = &MessageLine::newObject;
  objectFactoryMap[string("list")] = &MessageListAppend::newObject; // MessageListAppend factory creates any kind of list object
  objectFactoryMap[string(MessageLoadbang::getObjectLabel())] = &MessageLoadbang::newObject;
  objectFactoryMap[string(MessageLog::getObjectLabel())] = &MessageLog::newObject;
  objectFactoryMap[string(MessageLogicalAnd::getObjectLabel())] = &MessageLogicalAnd::newObject;
  objectFactoryMap[string(MessageLogicalOr::getObjectLabel())] = &MessageLogicalOr::newObject;
  objectFactoryMap[string(MessageMaximum::getObjectLabel())] = &MessageMaximum::newObject;
  objectFactoryMap[string(MessageMessageBox::getObjectLabel())] = &MessageMessageBox::newObject;
  objectFactoryMap[string(MessageMetro::getObjectLabel())] = &MessageMetro::newObject;
  objectFactoryMap[string(MessageMidiToFrequency::getObjectLabel())] = &MessageMidiToFrequency::newObject;
  objectFactoryMap[string(MessageMinimum::getObjectLabel())] = &MessageMinimum::newObject;
  objectFactoryMap[string(MessageModulus::getObjectLabel())] = &MessageModulus::newObject;
  objectFactoryMap[string(MessageMoses::getObjectLabel())] = &MessageMoses::newObject;
  objectFactoryMap[string(MessageMultiply::getObjectLabel())] = &MessageMultiply::newObject;
  objectFactoryMap[string(MessageNotein::getObjectLabel())] = &MessageNotein::newObject;
  objectFactoryMap[string(MessageNotEquals::getObjectLabel())] = &MessageNotEquals::newObject;
  objectFactoryMap[string(MessageOpenPanel::getObjectLabel())] = &MessageOpenPanel::newObject;
  objectFactoryMap[string(MessageOutlet::getObjectLabel())] = &MessageOutlet::newObject;
  objectFactoryMap[string(MessagePack::getObjectLabel())] = &MessagePack::newObject;
  objectFactoryMap[string(MessagePipe::getObjectLabel())] = &MessagePipe::newObject;
  objectFactoryMap[string(MessagePow::getObjectLabel())] = &MessagePow::newObject;
  objectFactoryMap[string(MessagePowToDb::getObjectLabel())] = &MessagePowToDb::newObject;
  objectFactoryMap[string(MessagePrint::getObjectLabel())] = &MessagePrint::newObject;
  objectFactoryMap[string(MessageRandom::getObjectLabel())] = &MessageRandom::newObject;
  objectFactoryMap[string(MessageReceive::getObjectLabel())] = &MessageReceive::newObject;
  objectFactoryMap[string("r")] = &MessageReceive::newObject;
  objectFactoryMap[string(MessageRemainder::getObjectLabel())] = &MessageRemainder::newObject;
  objectFactoryMap[string(MessageRmsToDb::getObjectLabel())] = &MessageRmsToDb::newObject;
  objectFactoryMap[string(MessageRoute::getObjectLabel())] = &MessageRoute::newObject;
  objectFactoryMap[string(MessageSamplerate::getObjectLabel())] = &MessageSamplerate::newObject;
  objectFactoryMap[string(MessageSelect::getObjectLabel())] = &MessageSelect::newObject;
  objectFactoryMap[string("sel")] = &MessageSelect::newObject;
  objectFactoryMap[string(MessageSend::getObjectLabel())] = &MessageSend::newObject;
  objectFactoryMap[string("s")] = &MessageSend::newObject;
  objectFactoryMap[string(MessageSine::getObjectLabel())] = &MessageSine::newObject;
  objectFactoryMap[string(MessageSoundfiler::getObjectLabel())] = &MessageSoundfiler::newObject;
  objectFactoryMap[string(MessageSpigot::getObjectLabel())] = &MessageSpigot::newObject;
  objectFactoryMap[string(MessageSqrt::getObjectLabel())] = &MessageSqrt::newObject;
  objectFactoryMap[string(MessageStripNote::getObjectLabel())] = &MessageStripNote::newObject;
  objectFactoryMap[string(MessageSubtract::getObjectLabel())] = &MessageSubtract::newObject;
  objectFactoryMap[string(MessageSwap::getObjectLabel())] = &MessageSwap::newObject;
  objectFactoryMap[string(MessageSwitch::getObjectLabel())] = &MessageSwitch::newObject;
  objectFactoryMap[string(MessageSymbol::getObjectLabel())] = &MessageSymbol::newObject;
  objectFactoryMap[string(MessageTable::getObjectLabel())] = &MessageTable::newObject;
  objectFactoryMap[string(MessageTableRead::getObjectLabel())] = &MessageTableRead::newObject;
  objectFactoryMap[string(MessageTableWrite::getObjectLabel())] = &MessageTableWrite::newObject;
  objectFactoryMap[string(MessageTangent::getObjectLabel())] = &MessageTangent::newObject;
  objectFactoryMap[string(MessageText::getObjectLabel())] = &MessageText::newObject;
  objectFactoryMap[string(MessageTimer::getObjectLabel())] = &MessageTimer::newObject;
  objectFactoryMap[string(MessageToggle::getObjectLabel())] = &MessageToggle::newObject;
  objectFactoryMap[string("tgl")] = &MessageToggle::newObject;
  objectFactoryMap[string(MessageTrigger::getObjectLabel())] = &MessageTrigger::newObject;
  objectFactoryMap[string("t")] = &MessageTrigger::newObject;
  objectFactoryMap[string(MessageUnpack::getObjectLabel())] = &MessageUnpack::newObject;
  objectFactoryMap[string(MessageUntil::getObjectLabel())] = &MessageUntil::newObject;
  objectFactoryMap[string(MessageValue::getObjectLabel())] = &MessageValue::newObject;
  objectFactoryMap[string("v")] = &MessageValue::newObject;
  objectFactoryMap[string(MessageWrap::getObjectLabel())] = &MessageWrap::newObject;
  
  // dsp objects
  objectFactoryMap[string(DspAdc::getObjectLabel())] = &DspAdc::newObject;
  objectFactoryMap[string(DspAdd::getObjectLabel())] = &DspAdd::newObject;
  objectFactoryMap[string(DspBandpassFilter::getObjectLabel())] = &DspBandpassFilter::newObject;
  objectFactoryMap[string(DspBang::getObjectLabel())] = &DspBang::newObject;
  objectFactoryMap[string(DspCatch::getObjectLabel())] = &DspCatch::newObject;
  objectFactoryMap[string(DspClip::getObjectLabel())] = &DspClip::newObject;
  objectFactoryMap[string(DspCosine::getObjectLabel())] = &DspCosine::newObject;
  objectFactoryMap[string(DspDac::getObjectLabel())] = &DspDac::newObject;
  objectFactoryMap[string(DspDelayRead::getObjectLabel())] = &DspDelayRead::newObject;
  objectFactoryMap[string(DspDelayWrite::getObjectLabel())] = &DspDelayWrite::newObject;
  objectFactoryMap[string(DspDivide::getObjectLabel())] = &DspDivide::newObject;
  objectFactoryMap[string(DspEnvelope::getObjectLabel())] = &DspEnvelope::newObject;
  objectFactoryMap[string(DspHighpassFilter::getObjectLabel())] = &DspHighpassFilter::newObject;
  objectFactoryMap[string(DspInlet::getObjectLabel())] = &DspInlet::newObject;
  objectFactoryMap[string(DspLine::getObjectLabel())] = &DspLine::newObject;
  objectFactoryMap[string(DspLog::getObjectLabel())] = &DspLog::newObject;
  objectFactoryMap[string(DspLowpassFilter::getObjectLabel())] = &DspLowpassFilter::newObject;
  objectFactoryMap[string(DspMinimum::getObjectLabel())] = &DspMinimum::newObject;
  objectFactoryMap[string(DspMultiply::getObjectLabel())] = &DspMultiply::newObject;
  objectFactoryMap[string(DspNoise::getObjectLabel())] = &DspNoise::newObject;
  objectFactoryMap[string(DspOsc::getObjectLabel())] = &DspOsc::newObject;
  objectFactoryMap[string(DspOutlet::getObjectLabel())] = &DspOutlet::newObject;
  objectFactoryMap[string(DspPhasor::getObjectLabel())] = &DspPhasor::newObject;
  objectFactoryMap[string(DspPrint::getObjectLabel())] = &DspPrint::newObject;
  objectFactoryMap[string(DspReceive::getObjectLabel())] = &DspReceive::newObject;
  objectFactoryMap[string("r~")] = &DspReceive::newObject;
  objectFactoryMap[string(DspReciprocalSqrt::getObjectLabel())] = &DspReciprocalSqrt::newObject;
  objectFactoryMap[string("q8_rsqrt~")] = &DspReciprocalSqrt::newObject;
  objectFactoryMap[string(DspRfft::getObjectLabel())] = &DspRfft::newObject;
  objectFactoryMap[string(DspRifft::getObjectLabel())] = &DspRifft::newObject;
  objectFactoryMap[string(DspSend::getObjectLabel())] = &DspSend::newObject;
  objectFactoryMap[string("s~")] = &DspSend::newObject;
  objectFactoryMap[string(DspSignal::getObjectLabel())] = &DspSignal::newObject;
  objectFactoryMap[string(DspSnapshot::getObjectLabel())] = &DspSnapshot::newObject;
  objectFactoryMap[string(DspSqrt::getObjectLabel())] = &DspSqrt::newObject;
  objectFactoryMap[string("q8_sqrt~")] = &DspSqrt::newObject;
  objectFactoryMap[string(DspSubtract::getObjectLabel())] = &DspSubtract::newObject;
  objectFactoryMap[string(DspTablePlay::getObjectLabel())] = &DspTablePlay::newObject;
  objectFactoryMap[string(DspTableRead::getObjectLabel())] = &DspTableRead::newObject;
  objectFactoryMap[string(DspTableRead4::getObjectLabel())] = &DspTableRead4::newObject;
  objectFactoryMap[string(DspThrow::getObjectLabel())] = &DspThrow::newObject;
  objectFactoryMap[string(DspVariableDelay::getObjectLabel())] = &DspVariableDelay::newObject;
}

ObjectFactoryMap::~ObjectFactoryMap() {
  
}

void ObjectFactoryMap::registerExternalObject(const char *objectLabel, MessageObject *(*newObject)(PdMessage *, PdGraph *)) {
  objectFactoryMap[string(objectLabel)] = newObject;
}

void ObjectFactoryMap::unregisterExternalObject(const char *objectLabel) {
  objectFactoryMap.erase(string(objectLabel));
}

MessageObject *ObjectFactoryMap::newObject(const char *objectLabel, PdMessage *initMessage, PdGraph *graph) {
  MessageObject *(*newObject)(PdMessage *, PdGraph *) = objectFactoryMap[string(objectLabel)];
  return (newObject != NULL) ? newObject(initMessage, graph) : NULL;
}
