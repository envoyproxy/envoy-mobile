#!/usr/bin/env python

import argparse
import base64
import json
import os
import shutil

try:
    from urllib.request import urlopen, Request, HTTPError
except ImportError:  # python 2
    from urllib2 import urlopen, Request, HTTPError

_USER_CREDS = os.environ.get("READWRITE_USER", "")
_KEY_CREDS = os.environ.get("READWRITE_API_KEY", "")
ENCODED_CREDENTIALS = base64.b64encode("{}:{}".format(_USER_CREDS, _KEY_CREDS).encode()).decode()

_ARTIFACT_HOST_URL = "https://oss.sonatype.org/service/local/staging"
_GROUP_ID = "io.envoyproxy.envoymobile"
_ARTIFACT_ID = "envoy"
_LOCAL_INSTALL_PATH = os.path.expanduser("~/.m2/repository/io/envoyproxy/envoymobile/{}".format(_ARTIFACT_ID))


def _resolve_name(file):
    file_name, file_extension = os.path.splitext(file)

    if file_extension[1:] == "asc":
        if file_name.endswith("pom.xml"):
            return ".pom", "asc"
        elif file_name.endswith("javadoc.jar"):
            return "-javadoc.jar", "asc"
        elif file_name.endswith("sources.jar"):
            return "-sources.jar", "asc"
        elif file_name.endswith(".aar"):
            return ".aar", file_extension[1:]
        elif file_name.endswith(".jar"):
            return ".jar", file_extension[1:]
    else:
        if file_name.endswith("pom"):
            return "", "pom"
        elif file_name.endswith("javadoc"):
            return "-javadoc", file_extension[1:]
        elif file_name.endswith("sources"):
            return "-sources", file_extension[1:]
        else:
            return "", file_extension[1:]


def _install_locally(version, files):
    path = "{}/{}".format(_LOCAL_INSTALL_PATH, version)

    if os.path.exists(path):
        shutil.rmtree(path)

    os.makedirs(path)

    for file in files:
        suffix, file_extension = _resolve_name(file)
        basename = "{name}-{version}{suffix}.{extension}".format(
            name=_ARTIFACT_ID,
            version=version,
            suffix=suffix,
            extension=file_extension
        )

        shutil.copyfile(file, os.path.join(path, basename))


def urlopen_retried(request, retries=3):
    try:
        return urlopen(request)
    except HTTPError as e:
        if retries > 0 and e.code >= 500:
            print("Retrying request for {}".format(request.url))
            return urlopen_retried(request, retries - 1)
        else:
            raise e


def _create_staging_repository(profile_id):
    try:
        url = os.path.join(_ARTIFACT_HOST_URL, "profiles/{}/start".format(profile_id))
        data = {
            'data': {
                'description': ''
            }
        }
        request = Request(url)
        request.add_header("Authorization", "Basic {}".format(ENCODED_CREDENTIALS))
        request.add_header("Content-Type", "application/json")
        request.get_method = lambda: "POST"
        request.add_data(json.dumps(data))

        response = json.load(urlopen_retried(request))
        staging_id = response["data"]["stagedRepositoryId"]
        print("staging id {} was created".format(staging_id))
        return staging_id
    except Exception as e:
        raise e


def _upload_files(staging_id, version, files):
    for file in files:
        # This will output "dist/envoy", ".aar" for "dist/envoy.aar
        suffix, file_extension = _resolve_name(file)
        basename = "{name}-{version}{suffix}.{extension}".format(
            name=_ARTIFACT_ID,
            version=version,
            suffix=suffix,
            extension=file_extension
        )

        artifact_url = os.path.join(
            _ARTIFACT_HOST_URL,
            "deployByRepositoryId/{}".format(staging_id),
            _GROUP_ID.replace('.', "/"),
            _ARTIFACT_ID,
            version,
            basename
        )

        try:
            with open(file, "rb") as f:
                request = Request(artifact_url, f.read())

            request.add_header("Authorization", "Basic {}".format(ENCODED_CREDENTIALS))
            request.add_header("Content-Type", "application/x-{extension}".format(extension=file_extension))
            request.get_method = lambda: "PUT"
            urlopen_retried(request, retries=20)
        except HTTPError as e:
            if e.code == 403:
                print("Ignoring duplicate upload for {}".format(artifact_url))
            else:
                raise e
        except Exception as e:
            raise e


def _close_staging_repository(profile_id, staging_id):
    url = os.path.join(_ARTIFACT_HOST_URL, "profiles/{}/finish".format(profile_id))
    data = {
        'data': {
            'stagedRepositoryId': staging_id,
            'description': ''
        }
    }

    try:
        request = Request(url)

        request.add_header("Authorization", "Basic {}".format(ENCODED_CREDENTIALS))
        request.add_header("Content-Type", "application/json")
        request.add_data(json.dumps(data))
        request.get_method = lambda: "PUT"
        urlopen_retried(request)
    except Exception as e:
        raise e


def _drop_staging_repository(staging_id):
    url = os.path.join(_ARTIFACT_HOST_URL, "/bulk/drop")
    data = {
        'data': {
            'stagedRepositoryId': [staging_id],
            'description': ''
        }
    }

    try:
        request = Request(url)

        request.add_header("Authorization", "Basic {}".format(ENCODED_CREDENTIALS))
        request.add_header("Content-Type", "application/json")
        request.add_data(json.dumps(data))
        request.get_method = lambda: "POST"
        urlopen_retried(request)
    except Exception as e:
        raise e


def _release_staging_repository(staging_id):
    url = os.path.join(_ARTIFACT_HOST_URL, "/bulk/promote")
    data = {
        'data': {
            'stagedRepositoryId': [staging_id],
            'description': ''
        }
    }

    try:
        request = Request(url)

        request.add_header("Authorization", "Basic {}".format(ENCODED_CREDENTIALS))
        request.add_header("Content-Type", "application/json")
        request.add_data(json.dumps(data))
        request.get_method = lambda: "POST"
        urlopen_retried(request)
    except Exception as e:
        raise e


def _build_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile_id", required=True,
                        help="""
                        The staging profile id of the sonatype repository target.
                        This is the id in the sonatype web ui. The REST api is:
                        curl -u {usr}:{psswrd} -H "Accept: application/json"
                        https://oss.sonatype.org//nexus/service/local/staging/profile_repositories
                        """)
    parser.add_argument("--version", default="LOCAL-SNAPSHOT",
                        help="""
                        The version of the artifact to be published. `LOCAL-SNAPSHOT` is defaulted
                        if the version is not set. This version should be consistent with the pom.xml
                        provided.
                        """)
    parser.add_argument("--local", nargs='?', const=True, default=False,
                        help="""
                        For installing artifacts into local maven. For now, we only support
                        installing to the path `~/.m2/repository/io/envoyproxy/envoymobile/`
                        """)
    parser.add_argument("--files", nargs="+", required=True,
                        help="""
                        Files to upload. The checklist for Envoy Mobile are:
                        Artifacts:
                            dist/envoy.aar
                            dist/envoy-pom.xml
                            dist/envoy-sources.jar
                            dist/envoy-javadoc.jar
                        GPG signed:
                            dist/envoy.aar.asc
                            dist/envoy-pom.xml.asc
                            dist/envoy-sources.jar.asc
                            dist/envoy-javadoc.jar.asc
                        """)
    return parser


if __name__ == "__main__":
    args = _build_parser().parse_args()

    if args.local:
        _install_locally(args.version, args.files)
    else:
        staging_id = ""

        try:
            staging_id = _create_staging_repository(args.profile_id)
        except:
            sys.exit(1, "Unable to create staging id")

        try:
            _upload_files(staging_id, args.version, args.files)
            # TODO: _close_staging_repository(args.profile_id, staging_id)
            # TODO: _release_staging_repository(staging_id)
        except:
            print("Unable to complete file upload. Will attempt to drop staging id: [{}]".format(staging_id))
            try:
                _drop_staging_repository(staging_id)
                sys.exit(1, "Dropping staging id: [{}] successful.".format(staging_id))
            except:
                sys.exit(1, "Dropping staging id: [{}] failed.".format(staging_id))
