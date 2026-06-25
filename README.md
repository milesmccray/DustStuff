# Dust Stuff

A community  collection of scripts and sprites for **Dustforce**, hosted as a static site built with Jekyll.

**Live site:** [duststuff.com](https://duststuff.com)

## What's here

- **Scripts** — gameplay, visual, audio, editor, and tool scripts 
- **Sprites** — character skins, sprite fixes, entity, and tilesets

## Built with

- [Jekyll](https://jekyllrb.com/) — static site generator
- Plain HTML/CSS/JS for the navbar and sprite preview modal
- Hosted on [GitHub Pages](https://pages.github.com/)

## Running locally

1. Install Ruby and Jekyll (see [Jekyll's installation docs](https://jekyllrb.com/docs/installation/) for your OS)
2. Clone this repo:
   ```bash
   git clone https://github.com/milesmccray/DustStuff.git
   cd DustStuff
   ```
3. Install dependencies:
   ```bash
   bundle install
   ```
4. Run the local server:
   ```bash
   bundle exec jekyll serve
   ```
5. Open [http://localhost:4000](http://localhost:4000) in your browser

## Project structure

```
DustStuff/
├── _data/              # scripts.json and sprites.json — all entry metadata
├── _includes/          # navbar
├── _layouts/           # page layout shell
├── assets/css/         # styles.css
├── js/                 # navbar + sprite preview logic
├── downloads/          # the actual script/sprite files
├── index.markdown       # homepage
├── scripts.md           # scripts listing page
└── sprites.md            # sprites listing page
```

## Contributing
If you notice a missing script/sprite or would like to add something...

**Message me on [Discord](https://discord.com/users/165860638080368640)**
