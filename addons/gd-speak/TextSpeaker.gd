class_name TextSpeaker extends Node

# -- Nodes --
var espeaker := ESpeaker.new()
var audio_player := AudioStreamPlayer.new()

# -- Settings --
# Examples (not coded here):
#var max_overlap: int = 1 # Create multiple AudioStreamPlayers to allow simultaneous TTS audio playback

# Language
## Full list of codes found [url=https://github.com/espeak-ng/espeak-ng/blob/master/docs/languages.md]here[/url].
@export_enum('en-us', 'en', 'es') var language_code: String = 'en-us': set = set_language_code


#region [ Engine ] 

# [TEST] Make sure language sets properly here when this class is created both in code and in editor
func _init(language: String = language_code) -> void:
	audio_player.stream = create_tts_stream()
	language_code = language
	add_child(audio_player) # Required to play audio

#endregion

#region [ TTS ] 

func speak(text: String = '') -> void:
	if text.is_empty():
		return
	if not audio_player.is_inside_tree():
		push_warning('TextSpeaker\'s AudioStreamPlayer Must be inside the scene tree to play audio.')
		return
	
	var raw_audio_data: PackedByteArray = espeaker.synthesize(text)
	if not raw_audio_data.is_empty():
		audio_player.stream.data = raw_audio_data
		audio_player.play()
	else:
		push_warning('No data from `espeaker.synthesize(text)`')

static func create_tts_stream() -> AudioStreamWAV:
	var stream_wav = AudioStreamWAV.new()
	stream_wav.format = AudioStreamWAV.FORMAT_16_BITS
	stream_wav.mix_rate = 22050
	stream_wav.stereo = false
	return stream_wav

#endregion

#region [ Getters + Setters ] 

func set_language_code(new_value: String) -> void:
	language_code = new_value
	espeaker.set_language(language_code)

#endregion
