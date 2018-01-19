#pragma once

#include "pch.h"

#include <string>
#include <vector>

#include "peer_view.h"
#include "buffer_capturer.h"
#include "directx_buffer_capturer.h"

// from ConfigParser
#include "structs.h"

#include "webrtc/base/sigslot.h"
#include "webrtc/base/json.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/test/fakeconstraints.h"

using namespace std;
using namespace webrtc;
using namespace rtc;
using namespace StreamingToolkit;
using namespace sigslot;

// Abstract PeerConductor
class PeerConductor : public PeerConnectionObserver,
	public CreateSessionDescriptionObserver,
	public DataChannelObserver
{
public:
	PeerConductor(int id,
		const string& name,
		shared_ptr<WebRTCConfig> webrtcConfig,
		scoped_refptr<PeerConnectionFactoryInterface> peerFactory,
		const function<void(const string&)>& sendFunc);

	~PeerConductor();

	// m_id, new_state, threadsafe per-instance 
	signal2<int, PeerConnectionInterface::IceConnectionState, sigslot::multi_threaded_local> SignalIceConnectionChange;

	// This callback transfers the ownership of the |desc|.
	// TODO(deadbeef): Make this take an std::unique_ptr<> to avoid confusion
	// around ownership.
	virtual void OnSuccess(SessionDescriptionInterface* desc) override;


	virtual void OnFailure(const string& error) override;

	// Triggered when the SignalingState changed.
	virtual void OnSignalingChange(
		PeerConnectionInterface::SignalingState new_state) override;

	// Triggered when renegotiation is needed. For example, an ICE restart
	// has begun.
	virtual void OnRenegotiationNeeded() override;

	// Called any time the IceConnectionState changes.
	//
	// Note that our ICE states lag behind the standard slightly. The most
	// notable differences include the fact that "failed" occurs after 15
	// seconds, not 30, and this actually represents a combination ICE + DTLS
	// state, so it may be "failed" if DTLS fails while ICE succeeds.
	virtual void OnIceConnectionChange(
		PeerConnectionInterface::IceConnectionState new_state) override;

	// Called any time the IceGatheringState changes.
	virtual void OnIceGatheringChange(
		PeerConnectionInterface::IceGatheringState new_state) override;

	// A new ICE candidate has been gathered.
	virtual void OnIceCandidate(const IceCandidateInterface* candidate) override;

	void OnAddStream(
		rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

	void OnRemoveStream(
		rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

	void OnDataChannel(
		rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;

	//  A data buffer was successfully received.
	virtual void OnMessage(const DataBuffer& buffer) override;

	virtual void OnStateChange() override;

	void HandlePeerMessage(const string& message);

	const int Id() const;

	const string& Name() const;

	const vector<scoped_refptr<webrtc::MediaStreamInterface>> Streams() const;

	shared_ptr<PeerView> View();

protected:
	// Allocates a buffer capturer for a single video track
	virtual unique_ptr<cricket::VideoCapturer> AllocateVideoCapturer() = 0;

private:
	void AllocatePeerConnection();

	int m_id;
	string m_name;
	shared_ptr<WebRTCConfig> m_webrtcConfig;
	scoped_refptr<PeerConnectionFactoryInterface> m_peerFactory;
	function<void(const string&)> m_sendFunc;
	scoped_refptr<PeerConnectionInterface> m_peerConnection;
	shared_ptr<PeerView> m_view;
	vector<scoped_refptr<webrtc::MediaStreamInterface>> m_peerStreams;

	// Names used for a IceCandidate JSON object.
	const char* kCandidateSdpMidName = "sdpMid";
	const char* kCandidateSdpMlineIndexName = "sdpMLineIndex";
	const char* kCandidateSdpName = "candidate";

	// Names used for stream labels.
	const char* kAudioLabel = "audio_label";
	const char* kVideoLabel = "video_label";
	const char* kStreamLabel = "stream_label";

	// Names used for a SessionDescription JSON object.
	const char* kSessionDescriptionTypeName = "type";
	const char* kSessionDescriptionSdpName = "sdp";
};