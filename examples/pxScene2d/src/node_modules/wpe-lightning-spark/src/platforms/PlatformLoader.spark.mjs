import SparkPlatform from "./spark/SparkPlatform.mjs";
export default class PlatformLoader {
    static load(options) {
        return SparkPlatform;
    }
}