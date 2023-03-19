# `Plotly.js` bundle creation

## Reason for that
[As you can see](https://github.com/plotly/plotly.js/tree/master/dist), default plotly bundle is too big (3.7 MB) for ESP32 SPIFFS but we can create custom


## Updating `npm` & `node.js`
### `nvm` installation
- `curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.3/install.sh | bash`
- `nvm ls-remote` to show available version of node.js

## Update
- `nvm install v19.8.1`
- `npm install -g npm@latest`


## Custom bundle
- link: [Custom bundle](https://github.com/plotly/plotly.js/blob/master/CUSTOM_BUNDLE.md)
### Steps
- ~~download plotly.js release, for example [v2.20.0](https://github.com/plotly/plotly.js/archive/refs/tags/v2.20.0.zip) and unzip it~~
- Install plotly.js, move to plotly.js folder then install plotly.js dependencies:
```Bash
cd web/plotly_bundle/
npm i plotly.js@v2.20.0
cp -r ~/node_modules/plotly.js/ plotly.js
cd plotly.js
npm i
```
- in `plotly.js` directory run command to create `barpolar` bundle: 
```Bash
npm run custom-bundle -- --minified --out barpolar --traces barpolar --transforms none
```
- print bundled file size: `du -h dist/plotly-barpolar.min.js` (around 960K)

- in `plotly.js` directory run command to copy bundle into `src/` dir:
```Bash
cp dist/plotly-barpolar.min.js ../../src/lib_plotly-$(node -p "require('./package.json').version")-bp.min.js
```