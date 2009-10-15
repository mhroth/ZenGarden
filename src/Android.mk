LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libzengarden
LOCAL_SRC_FILES := \
./DspAdd.cpp \
./DspAdc.cpp \
./DspBandpassFilter.cpp \
./DspBinaryOperationObject.cpp \
./DspClip.cpp \
./DspDac.cpp \
./DspDelayRead.cpp \
./DspDelayWrite.cpp \
./DspDivide.cpp \
./DspInletOutlet.cpp \
./DspInputDspOutputObject.cpp \
./DspInputMessageOutputObject.cpp \
./DspMessageInputDspOutputObject.cpp \
./DspMessageInputMessageOutputObject.cpp \
./DspLine.cpp \
./DspLowpassFilter.cpp \
./DspMultiply.cpp \
./DspOsc.cpp \
./DspOutputObject.cpp \
./DspNoise.cpp \
./DspPhasor.cpp \
./DspSendReceive.cpp \
./DspSig.cpp \
./DspSubtract.cpp \
./DspTable.cpp \
./DspTableRead.cpp \
./DspVariableDelay.cpp \
./DspVCF.cpp \
./List.cpp \
./MessageAbsoluteValue.cpp \
./MessageAdd.cpp \
./MessageArcTangent2.cpp \
./MessageBang.cpp \
./MessageBinaryOperationObject.cpp \
./MessageChange.cpp \
./MessageClip.cpp \
./MessageCosine.cpp \
./MessageDbToPow.cpp \
./MessageDbToRms.cpp \
./MessageDelay.cpp \
./MessageDivide.cpp \
./MessageElement.cpp \
./MessageEnv.cpp \
./MessageExp.cpp \
./MessageEqualsEquals.cpp \
./MessageExternalSend.cpp \
./MessageFloat.cpp \
./MessageGreaterThan.cpp \
./MessageInletOutlet.cpp \
./MessageInputDspOutputObject.cpp \
./MessageInputMessageOutputObject.cpp \
./MessageLessThan.cpp \
./MessageLessThanOrEqualTo.cpp \
./MessageLine.cpp \
./MessageLoadbang.cpp \
./MessageMessageBox.cpp \
./MessageMetro.cpp \
./MessageMidiToFrequency.cpp \
./MessageModulus.cpp \
./MessageMoses.cpp \
./MessageMultiply.cpp \
./MessagePack.cpp \
./MessagePow.cpp \
./MessagePowToDb.cpp \
./MessagePrint.cpp \
./MessageOutputObject.cpp \
./MessageRandom.cpp \
./MessageRmsToDb.cpp \
./MessageRoute.cpp \
./MessageSelect.cpp \
./MessageSendReceive.cpp \
./MessageSine.cpp \
./MessageSpigot.cpp \
./MessageSoundfiler.cpp \
./MessageSqrt.cpp \
./MessageSubtract.cpp \
./MessageSwap.cpp \
./MessageSymbol.cpp \
./MessageTangent.cpp \
./MessageToggle.cpp \
./MessageTrigger.cpp \
./MessageUnaryOperationObject.cpp \
./MessageUnpack.cpp \
./PdGraph.cpp \
./PdMessage.cpp \
./PdObject.cpp \
./RemoteBufferObject.cpp \
./RemoteBufferReceiverObject.cpp \
./StaticUtils.cpp \
./TextObject.cpp \

LOCAL_STATIC_LIBRARIES := sndfile
#LOCAL_SHARED_LIBRARIES := sndfile

LOCAL_CFLAGS := \
-I$(LOCAL_PATH)/../../libsndfile-1.0.20/src \
-L$(LOCAL_PATH)/../../libsndfile-1.0.20/src/libs/

include $(BUILD_SHARED_LIBRARY)
