Overview:-
Steganography is the art and science of hiding information within other non-secret, innocent-looking data—like images, audio, or video files—so that only the intended recipient knows of its existence.

This project demonstrates how to implement basic image-based steganography using Python. It allows you to embed text messages into image files and retrieve them later without altering the visual appearance of the image.
Features:-
Hide secret messages inside images (PNG)

 Extract hidden messages from stego-images

 Minimal visual distortion of the image

 CLI-based user interface
Encoding:
The message is converted into binary.

Each bit is embedded into the least significant bits (LSBs) of the image pixels.

The modified image (stego-image) looks identical to the original.

Decoding:
The LSBs of the pixels are extracted.

The bits are grouped and converted back into characters.

The hidden message is revealed.
Real-Time Applications
Secure communication in hostile environments

Digital watermarking

Preventing unauthorized access to sensitive information

Anti-piracy and copyright enforcement
