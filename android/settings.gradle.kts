// Escala de Notas — proyecto Android Studio / F-Droid. La interfaz vive en ../ui (la misma de escritorio).
pluginManagement { repositories { google(); mavenCentral(); gradlePluginPortal() } }
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories { google(); mavenCentral() }
}
rootProject.name = "EscalaNotas"
