#!/bin/bash

# Define the output palette image name
PALETTE_IMG="palette.png"

# Start with a blank image, or remove previous one
> $PALETTE_IMG

# Read colors from text file and append them to the palette image
while IFS= read -r color; do
  echo $color
  # Skip empty lines or comments
  if [[ -z "$color" ]]; then
    echo here
    continue
  fi
  # If the palette image doesn't exist, create the first pixel
  if [[ ! -f "$PALETTE_IMG" ]]; then
      convert xc:"$color" "$PALETTE_IMG"
  else
      # Otherwise, append the new color pixel
      convert "$PALETTE_IMG" xc:"$color" +append "$PALETTE_IMG"
  fi
done < my_palette.txt

echo "Created palette image: $PALETTE_IMG"
