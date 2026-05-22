input="$1"
output="output/$(basename "${input%.*}").gif"

ffmpeg -i "$input" -vf "crop=iw-380:ih:180:0,fps=10,scale=1080:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" "$output"
