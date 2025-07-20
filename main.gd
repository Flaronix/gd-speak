extends Node2D
@onready var line_edit = $LineEdit
@onready var audio_player = $AudioStreamPlayer
@onready var espeak_node = $ESpeakNode
@onready var spanish_toggle = $SpanishToggle 
func _ready():
	if spanish_toggle.button_pressed:
		espeak_node.set_language("es")
	else:
		espeak_node.set_language("en")
func _on_button_pressed():
	var text = line_edit.text
	if text.is_empty():
		return
	var raw_audio_data = espeak_node.synthesize(text)
	if not raw_audio_data.is_empty():
		var audio_stream = AudioStreamWAV.new()
		audio_stream.format = AudioStreamWAV.FORMAT_16_BITS 
		audio_stream.mix_rate = 22050 
		audio_stream.stereo = false 
		audio_stream.data = raw_audio_data
		audio_player.stream = audio_stream
		audio_player.play()
	else:
		print("Error: La síntesis de voz no ha devuelto datos.")
func _on_SpanishToggle_toggled(button_pressed):
	var lang_to_set = "en"
	if button_pressed:
		lang_to_set = "es"
	print("GDScript: Intentando cambiar idioma a: " + lang_to_set)
	espeak_node.set_language(lang_to_set)
	if button_pressed:
		print("GDScript: Idioma cambiado a: Español")
	else:
		print("GDScript: Idioma cambiado a: Inglés")
