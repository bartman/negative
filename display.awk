#! /usr/bin/awk -f

# Look for layer ids -> labels.
BEGIN {
  NUM_LAYERS=0
}
/^[\t ]*id="layer/ {
  gsub(/^[\t ]*id="/,""); gsub(/"/,"");
  LAYERID[NUM_LAYERS]=$0;
}
/^[\t ]*inkscape:label="/ {
  gsub(/^[\t ]*inkscape:label="/,""); gsub(/"/,"");
  if (LAYERID[NUM_LAYERS] != "")
    LAYERNAME[NUM_LAYERS++]=$0;
}
END {
  # Now, for each layer, print out it and the following layers which are a 
  # substring of it (inkscape writes out layers backwards).
  for (i = NUM_LAYERS-1; i > 0; i--) {
    LINE=""
    for (j = i; j < NUM_LAYERS; j++) {
      if (substr(LAYERNAME[i], 0, length(LAYERNAME[j])) == LAYERNAME[j])
	LINE = LINE "," LAYERID[j];
    }
    LINE = LAYERID[0] LINE;
    print LINE
  }
}
