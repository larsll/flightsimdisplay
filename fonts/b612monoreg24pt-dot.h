const uint8_t B612Mono_Regular24ptDotBitmaps[] PROGMEM = {
  0x03, 0xFC, 0x00, 0xFF, 0xF0, 0x1F, 0xFF, 0x83, 0xFF, 0xFC, 0x3F, 0xFF,
  0xC7, 0xFF, 0xFE, 0x7F, 0xFF, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0x7F, 0xFF, 0xE7, 0xFF, 0xFE, 0x3F,
  0xFF, 0xE3, 0xFF, 0xFC, 0x1F, 0xFF, 0x80, 0xFF, 0xF0, 0x03, 0xFC, 0x00 };

const GFXglyph B612Mono_Regular24ptDotGlyphs[] PROGMEM = {
  {     0,  20,  19,  31,    4,  -25 } }; // 0x25CF

const GFXfont B612Mono_Regular24ptDot PROGMEM = {
  (uint8_t  *)B612Mono_Regular24ptDotBitmaps,
  (GFXglyph *)B612Mono_Regular24ptDotGlyphs,
  0x2E, 0x2E, 57 };
// Referencing black circle (0x25CF) here as normal dot (0x2E)
// Approx. 62 bytes
