all: readme-en.html readme-en-flat.html readme-fr.html readme-fr-flat.html

readme-en.html: readme-en.md
	reveal-md readme-en.md --static . --theme csgames
	mv index.html readme-en.html

readme-en-flat.html: readme-en.md
	pandoc -f markdown_github -t html readme-en.md -o readme-en-flat.html

readme-fr.html: readme-fr.md
	reveal-md readme-fr.md --static . --theme csgames
	mv index.html readme-fr.html

readme-fr-flat.html: readme-fr.md
	pandoc -f markdown_github -t html readme-fr.md -o readme-fr-flat.html

clean:
	rm *.html
	rm -rf css/ js/ lib/ plugin/
