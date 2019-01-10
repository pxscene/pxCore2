px.import(px.appQueryParams.module)
.then(x => {
  module.exports.test_access_url = () => {
    return new Promise(resolve => {
      resolve(x.request(px.appQueryParams.url, () => {}).blocked);
    });
  };
});