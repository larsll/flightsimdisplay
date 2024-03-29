const uint8_t B612Mono_Regular24ptDegBitmaps[] PROGMEM = {
  0x0F, 0x81, 0xFF, 0x1F, 0xFC, 0xE0, 0xEE, 0x03, 0xE0, 0x0F, 0x00, 0x78,
  0x03, 0xE0, 0x3B, 0x83, 0x9F, 0xFC, 0x7F, 0xC0, 0xF8, 0x00 };

const GFXglyph B612Mono_Regular24ptDegGlyphs[] PROGMEM = {
  {     0,  13,  13,  31,    5,  -35 } }; // 0xB0

const GFXfont B612Mono_Regular24ptDeg PROGMEM = {
  (uint8_t  *)B612Mono_Regular24ptDegBitmaps,
  (GFXglyph *)B612Mono_Regular24ptDegGlyphs,
  0xB0, 0xB0, 57 };

// Approx. 36 bytes
