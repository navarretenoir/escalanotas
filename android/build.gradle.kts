// Escala de Notas para Android 4.0 en adelante (desarrollada por Javiera Navarrete). Sin dependencias externas.
plugins { id("com.android.application") version "9.4.1" }

val assetsUi = layout.buildDirectory.dir("assets-ui").get().asFile
val prepararUi by tasks.registering(Sync::class) {
    into(assetsUi)
    from(rootDir.resolve("../ui")) { into("ui"); exclude("*.bak*") }
}

android {
    namespace = "io.github.navarretenoir.escalanotas"
    compileSdk { version = release(36) { minorApiLevel = 1 } }
    defaultConfig {
        applicationId = "io.github.navarretenoir.escalanotas"
        minSdk = 14          // Android 4.0
        targetSdk = 36
        versionCode = 1
        versionName = "1.0"
    }
    sourceSets["main"].apply {
        manifest.srcFile("AndroidManifest.xml")
        java.srcDirs("src")
        res.srcDirs("res")
        assets.srcDirs(assetsUi)
    }
    buildTypes { release { isMinifyEnabled = false } }
    compileOptions { sourceCompatibility = JavaVersion.VERSION_11; targetCompatibility = JavaVersion.VERSION_11 }
    lint { abortOnError = false; checkOnly += setOf("NewApi", "InlinedApi"); textReport = true }
}
tasks.named("preBuild") { dependsOn(prepararUi) }
