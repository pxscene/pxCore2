px.import('http').then(http => {
  http.get(`http://deelay.me/5000/http://example.com`, () => {
    process.env.TEST_HTTP_5S_VAL = px.appQueryParams.val;
  });
});
